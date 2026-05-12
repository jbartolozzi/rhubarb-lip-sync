#include "Rhubarb.h"

#include "lib/rhubarbLib.h"
#include "core/Shape.h"
#include "core/appInfo.h"
#include "recognition/BlendshapeTimeline.h"
#include "recognition/PocketSphinxRecognizer.h"
#include "recognition/PhoneticRecognizer.h"
#include "recognition/pocketSphinxTools.h"
#include "animation/targetShapeSet.h"
#include "tools/progress.h"
#include "tools/parallel.h"
#include "tools/platformTools.h"

#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <filesystem>
#include <memory>
#include <sstream>
#include <string>

#if defined(__APPLE__)
    #include <os/log.h>
#endif

namespace {

// Bridge from the C ABI into a logging sink the host app can observe.
// On Apple platforms we route through os_log under the same subsystem
// the Swift app uses, so entries show up in `log stream`, Console.app,
// and any os.Logger-based viewer. We additionally mirror to stderr for
// Xcode console / terminal launches. On non-Apple platforms, stderr is
// the only sink — matches upstream rhubarb's logging behavior.
void log_to_host(const char* fmt, ...) {
    char buffer[512];
    va_list args;
    va_start(args, fmt);
    std::vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

#if defined(__APPLE__)
    static os_log_t logger =
        os_log_create("studiorischio.Mocap-Studio", "rhubarb-cabi");
    // Mark the formatted text public so it isn't redacted as <private>
    // when viewed via `log stream` from outside the app's signing scope.
    os_log_info(logger, "%{public}s", buffer);
#endif

    std::fprintf(stderr, "%s\n", buffer);
}

}

namespace {

thread_local std::string g_lastError;

void setLastError(const char* what) noexcept {
    try {
        g_lastError = what ? what : "";
    } catch (...) {
        // If even string assignment fails we can't do anything useful
    }
}

ShapeSet parseShapeSet(const char* extendedShapes) {
    ShapeSet result(ShapeConverter::get().getBasicShapes());
    if (!extendedShapes) return result;
    for (const char* p = extendedShapes; *p; ++p) {
        Shape shape = ShapeConverter::get().parse(std::string(1, *p));
        result.insert(shape);
    }
    return result;
}

std::unique_ptr<Recognizer> createRecognizer(
    RhubarbRecognizer kind,
    const RhubarbOptions& opts
) {
    switch (kind) {
        case RHUBARB_RECOGNIZER_POCKET_SPHINX: {
            BlendshapeVadParams vad;
            // Only attach a blendshape track when both the data and the
            // toggle are present. Without the toggle the track is
            // ignored even if it was supplied — keeps the C ABI's
            // blendshape fields useful for future passes (viseme fusion,
            // etc.) that don't want VAD gating.
            if (opts.vad_use_blendshape_mask
                && opts.blendshape_frame_count > 0
                && opts.blendshape_timestamps != nullptr
                && opts.blendshape_values != nullptr)
            {
                vad.track = std::make_shared<BlendshapeTimeline>(
                    opts.blendshape_timestamps,
                    opts.blendshape_values,
                    opts.blendshape_frame_count,
                    opts.blendshape_values_per_frame);
                vad.jawOpenThreshold = opts.vad_jaw_open_threshold;
                vad.minSilenceMs = opts.vad_min_silence_ms;
            }
            return std::make_unique<PocketSphinxRecognizer>(std::move(vad));
        }
        case RHUBARB_RECOGNIZER_PHONETIC:
            return std::make_unique<PhoneticRecognizer>();
    }
    throw std::invalid_argument("Unknown recognizer enum value");
}

char shapeToChar(Shape shape) {
    std::ostringstream stream;
    stream << shape;
    const std::string s = stream.str();
    return s.empty() ? '?' : s.front();
}

} // namespace

extern "C" {

void rhubarb_default_options(RhubarbOptions* options) {
    if (!options) return;
    options->recognizer = RHUBARB_RECOGNIZER_POCKET_SPHINX;
    options->dialog = nullptr;
    options->extended_shapes = "GHX";
    options->framerate = 0;
    options->thread_count = 0;
    options->blendshape_timestamps = nullptr;
    options->blendshape_values = nullptr;
    options->blendshape_frame_count = 0;
    options->blendshape_values_per_frame = 0;
    options->vad_use_blendshape_mask = 0;
    options->vad_jaw_open_threshold = 0.05f;
    options->vad_min_silence_ms = 200;
}

void rhubarb_set_resource_directory(const char* path) {
    if (!path) return;
    try {
        setBinDirectory(std::filesystem::u8path(path));
    } catch (const std::exception& e) {
        setLastError(e.what());
    } catch (...) {
        setLastError("Unknown error setting resource directory");
    }
}

RhubarbStatus rhubarb_animate(
    const char* audio_path,
    const RhubarbOptions* options,
    RhubarbCue** out_cues,
    size_t* out_count
) {
    if (!audio_path || !out_cues || !out_count) {
        setLastError("audio_path, out_cues, and out_count must be non-null");
        return RHUBARB_ERROR_INVALID_ARGUMENT;
    }
    *out_cues = nullptr;
    *out_count = 0;

    RhubarbOptions defaults;
    rhubarb_default_options(&defaults);
    const RhubarbOptions& opts = options ? *options : defaults;

    // Step 1 of blendshape integration: just confirm what reached us
    // across the Swift -> C ABI -> C++ boundary. Real consumption of
    // these values comes in later steps (VAD intersection, viseme
    // fusion, etc.). Routed through log_to_host() so the line appears
    // in the host's unified log on Apple platforms, not just stderr.
    if (opts.blendshape_frame_count > 0
            && opts.blendshape_timestamps != nullptr
            && opts.blendshape_values != nullptr) {
        const double firstT = opts.blendshape_timestamps[0];
        const double lastT =
            opts.blendshape_timestamps[opts.blendshape_frame_count - 1];
        log_to_host(
            "rhubarb: received %zu blendshape frames "
            "(%zu values each, %.3fs..%.3fs)",
            opts.blendshape_frame_count,
            opts.blendshape_values_per_frame,
            firstT,
            lastT);
    } else {
        log_to_host("rhubarb: no blendshape data supplied");
    }

    try {
        const std::filesystem::path audioFile = std::filesystem::u8path(audio_path);
        if (!std::filesystem::exists(audioFile)) {
            setLastError(("Audio file not found: " + audioFile.u8string()).c_str());
            return RHUBARB_ERROR_FILE_NOT_FOUND;
        }

        auto recognizer = createRecognizer(opts.recognizer, opts);
        const ShapeSet shapeSet = parseShapeSet(
            opts.extended_shapes ? opts.extended_shapes : "GHX");

        boost::optional<std::string> dialog;
        if (opts.dialog && *opts.dialog) {
            dialog = std::string(opts.dialog);
        }

        const int threadCount =
            opts.thread_count > 0 ? opts.thread_count : getProcessorCoreCount();

        NullProgressSink progressSink;

        const JoiningContinuousTimeline<Shape> animation = animateWaveFile(
            audioFile,
            dialog,
            *recognizer,
            shapeSet,
            threadCount,
            progressSink,
            opts.framerate);

        size_t count = 0;
        for (auto it = animation.begin(); it != animation.end(); ++it) ++count;

        if (count == 0) {
            return RHUBARB_OK;
        }

        auto* buffer = static_cast<RhubarbCue*>(
            std::calloc(count, sizeof(RhubarbCue)));
        if (!buffer) {
            setLastError("Out of memory allocating cue buffer");
            return RHUBARB_ERROR_INTERNAL;
        }

        size_t i = 0;
        for (const auto& timedShape : animation) {
            buffer[i].start_seconds =
                std::chrono::duration<double>(timedShape.getStart()).count();
            buffer[i].end_seconds =
                std::chrono::duration<double>(timedShape.getEnd()).count();
            buffer[i].shape = shapeToChar(timedShape.getValue());
            ++i;
        }

        *out_cues = buffer;
        *out_count = count;
        return RHUBARB_OK;

    } catch (const std::invalid_argument& e) {
        setLastError(e.what());
        return RHUBARB_ERROR_INVALID_ARGUMENT;
    } catch (const std::filesystem::filesystem_error& e) {
        setLastError(e.what());
        return RHUBARB_ERROR_FILE_NOT_FOUND;
    } catch (const std::exception& e) {
        setLastError(e.what());
        return RHUBARB_ERROR_RECOGNITION_FAILED;
    } catch (...) {
        setLastError("Unknown error during animation");
        return RHUBARB_ERROR_INTERNAL;
    }
}

void rhubarb_free_cues(RhubarbCue* cues) {
    std::free(cues);
}

const char* rhubarb_last_error(void) {
    return g_lastError.c_str();
}

const char* rhubarb_version(void) {
    return appVersion.c_str();
}

} // extern "C"
