#include "Rhubarb.h"

#include "lib/rhubarbLib.h"
#include "core/Shape.h"
#include "core/appInfo.h"
#include "recognition/PocketSphinxRecognizer.h"
#include "recognition/PhoneticRecognizer.h"
#include "animation/targetShapeSet.h"
#include "tools/progress.h"
#include "tools/parallel.h"
#include "tools/platformTools.h"

#include <chrono>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <filesystem>
#include <memory>
#include <sstream>
#include <string>

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

std::unique_ptr<Recognizer> createRecognizer(RhubarbRecognizer kind) {
    switch (kind) {
        case RHUBARB_RECOGNIZER_POCKET_SPHINX:
            return std::make_unique<PocketSphinxRecognizer>();
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

    try {
        const std::filesystem::path audioFile = std::filesystem::u8path(audio_path);
        if (!std::filesystem::exists(audioFile)) {
            setLastError(("Audio file not found: " + audioFile.u8string()).c_str());
            return RHUBARB_ERROR_FILE_NOT_FOUND;
        }

        auto recognizer = createRecognizer(opts.recognizer);
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
