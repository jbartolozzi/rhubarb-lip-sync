#include "whisperTranscribe.h"
#include <whisper.h>
#include <algorithm>
#include <cctype>
#include <memory>
#include "audio/SampleRateConverter.h"
#include "logging/logging.h"

using std::string;
using std::vector;
using std::filesystem::path;
using boost::optional;

constexpr int whisperSampleRate = 16000;

static void whisperLogCallback(ggml_log_level level, const char* text, void*) {
	if (!text) return;
	switch (level) {
		case GGML_LOG_LEVEL_ERROR:
			logging::error(text);
			break;
		case GGML_LOG_LEVEL_WARN:
			logging::warn(text);
			break;
		default:
			logging::debug(text);
			break;
	}
}

static vector<float> audioClipToFloat(const AudioClip& audioClip) {
	auto resampled = audioClip.clone() | resample(whisperSampleRate);
	vector<float> result;
	result.reserve(resampled->size());
	for (float sample : *resampled) {
		result.push_back(sample);
	}
	return result;
}

string transcribeWithWhisper(
	const AudioClip& audioClip,
	const path& modelPath,
	const optional<string>& dialog,
	int maxThreadCount,
	ProgressSink& progressSink
) {
	logging::info("Transcribing with Whisper...");

	whisper_log_set(whisperLogCallback, nullptr);

	whisper_context_params cparams = whisper_context_default_params();
	whisper_context* ctx = whisper_init_from_file_with_params(
		modelPath.string().c_str(), cparams);
	if (!ctx) {
		throw std::runtime_error(
			"Failed to load Whisper model from: " + modelPath.string());
	}

	auto cleanup = [](whisper_context* c) { whisper_free(c); };
	std::unique_ptr<whisper_context, decltype(cleanup)> ctxGuard(ctx, cleanup);

	vector<float> samples = audioClipToFloat(audioClip);

	whisper_full_params params = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
	params.n_threads = std::max(1, maxThreadCount);
	params.token_timestamps = false;
	params.split_on_word = false;
	params.no_context = true;
	params.language = "en";
	params.suppress_nst = true;
	if (dialog) {
		params.initial_prompt = dialog->c_str();
	}
	params.print_progress = false;
	params.print_timestamps = false;
	params.print_special = false;
	params.print_realtime = false;

	int result = whisper_full(ctx, params, samples.data(),
		static_cast<int>(samples.size()));
	if (result != 0) {
		throw std::runtime_error("Whisper inference failed with error code: "
			+ std::to_string(result));
	}

	progressSink.reportProgress(1.0);

	// Concatenate all segments into a single transcript
	string transcript;
	int nSegments = whisper_full_n_segments(ctx);
	for (int i = 0; i < nSegments; ++i) {
		const char* text = whisper_full_get_segment_text(ctx, i);
		if (text) {
			if (!transcript.empty()) transcript += ' ';
			transcript += text;
		}
	}

	logging::infoFormat("Whisper transcript: \"{}\"", transcript);
	return transcript;
}
