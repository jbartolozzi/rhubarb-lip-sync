#include "WhisperRecognizer.h"
#include <whisper.h>
#include <algorithm>
#include <cctype>
#include <regex>
#include "cmuDictionary.h"
#include "g2p.h"
#include "audio/DcOffset.h"
#include "audio/SampleRateConverter.h"
#include "audio/voiceActivityDetection.h"
#include "tools/platformTools.h"
#include "tools/progress.h"
#include "logging/logging.h"
#include "time/BoundedTimeline.h"
#include "time/centiseconds.h"

using std::string;
using std::vector;
using std::filesystem::path;
using boost::optional;

constexpr int whisperSampleRate = 16000; // WHISPER_SAMPLE_RATE

// Suppress whisper.cpp's internal logging unless debug logging is enabled
static void whisperLogCallback(ggml_log_level level, const char* text, void*) {
	if (!text) return;
	// Route whisper's log output through rhubarb's logging system
	switch (level) {
		case GGML_LOG_LEVEL_ERROR:
			logging::error(text);
			break;
		case GGML_LOG_LEVEL_WARN:
			logging::warn(text);
			break;
		default:
			// Debug/info from whisper only shown at trace level
			logging::debug(text);
			break;
	}
}

WhisperRecognizer::WhisperRecognizer(const path& modelPath) :
	modelPath(modelPath)
{}

path WhisperRecognizer::resolveModelPath() const {
	if (!modelPath.empty()) {
		return modelPath;
	}
	// Look in the default resource directory next to the binary
	return getBinDirectory() / "res" / "whisper" / "ggml-tiny.bin";
}

// Converts an AudioClip to 16kHz mono float32 samples (Whisper's required format)
static vector<float> audioClipToFloat32(const AudioClip& audioClip) {
	// Resample to 16kHz
	auto resampled = audioClip.clone() | resample(whisperSampleRate);

	// Collect float samples directly from the AudioClip iterator
	vector<float> result;
	result.reserve(resampled->size());
	for (float sample : *resampled) {
		result.push_back(sample);
	}
	return result;
}

// Strip punctuation and normalize a word from Whisper output
static string normalizeWord(const string& word) {
	string result;
	for (char c : word) {
		if (std::isalpha(static_cast<unsigned char>(c)) || c == '\'') {
			result += std::tolower(static_cast<unsigned char>(c));
		}
	}
	return result;
}

struct RecognizedWord {
	string text;
	centiseconds start;
	centiseconds end;
};

// Run Whisper inference and extract word-level timestamps
static vector<RecognizedWord> recognizeWords(
	const vector<float>& audioSamples,
	const path& modelPath,
	const optional<string>& dialog,
	int maxThreadCount,
	ProgressSink& progressSink
) {
	logging::info("Loading Whisper model...");

	// Redirect whisper.cpp logging through rhubarb's logging system
	whisper_log_set(whisperLogCallback, nullptr);

	whisper_context_params cparams = whisper_context_default_params();
	whisper_context* ctx = whisper_init_from_file_with_params(
		modelPath.string().c_str(), cparams);
	if (!ctx) {
		throw std::runtime_error(
			"Failed to load Whisper model from: " + modelPath.string());
	}

	// Ensure cleanup on exit
	auto cleanup = [](whisper_context* c) { whisper_free(c); };
	std::unique_ptr<whisper_context, decltype(cleanup)> ctxGuard(ctx, cleanup);

	logging::info("Running Whisper inference...");

	whisper_full_params params = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
	params.n_threads = std::max(1, maxThreadCount);
	params.token_timestamps = true;
	params.split_on_word = true;
	params.max_len = 1; // Force per-word segmentation
	params.no_context = true;
	params.language = "en";
	params.suppress_nst = true;
	// Use dialog text as initial prompt to bias recognition toward expected words
	if (dialog) {
		params.initial_prompt = dialog->c_str();
	}
	params.print_progress = false;
	params.print_timestamps = false;
	params.print_special = false;
	params.print_realtime = false;

	int result = whisper_full(ctx, params, audioSamples.data(),
		static_cast<int>(audioSamples.size()));
	if (result != 0) {
		throw std::runtime_error("Whisper inference failed with error code: "
			+ std::to_string(result));
	}

	progressSink.reportProgress(1.0);

	// Extract segments as words, filtering out likely non-speech
	vector<RecognizedWord> words;
	int nSegments = whisper_full_n_segments(ctx);
	for (int i = 0; i < nSegments; ++i) {
		// Skip segments with high non-speech probability (likely silence/noise)
		float noSpeechProb = whisper_full_get_segment_no_speech_prob(ctx, i);
		if (noSpeechProb > 0.6f) continue;

		const char* text = whisper_full_get_segment_text(ctx, i);
		int64_t t0 = whisper_full_get_segment_t0(ctx, i); // in 10ms units
		int64_t t1 = whisper_full_get_segment_t1(ctx, i);

		string wordText = normalizeWord(text ? text : "");
		if (wordText.empty()) continue;

		words.push_back({
			wordText,
			centiseconds(static_cast<int>(t0)),
			centiseconds(static_cast<int>(t1))
		});
	}

	logging::debugFormat("Whisper recognized {} words", words.size());
	return words;
}

// Distribute phones evenly across a word's time range
static void distributePhones(
	const vector<Phone>& phones,
	centiseconds wordStart,
	centiseconds wordEnd,
	BoundedTimeline<Phone>& result
) {
	if (phones.empty()) return;

	const centiseconds wordDuration = wordEnd - wordStart;
	if (wordDuration <= centiseconds(0)) return;

	const int phoneCount = static_cast<int>(phones.size());

	for (int i = 0; i < phoneCount; ++i) {
		centiseconds phoneStart = wordStart +
			centiseconds(wordDuration.count() * i / phoneCount);
		centiseconds phoneEnd = wordStart +
			centiseconds(wordDuration.count() * (i + 1) / phoneCount);

		if (phoneStart < phoneEnd) {
			result.set(phoneStart, phoneEnd, phones[i]);
		}
	}
}

BoundedTimeline<Phone> WhisperRecognizer::recognizePhones(
	const AudioClip& inputAudioClip,
	optional<string> dialog,
	int maxThreadCount,
	ProgressSink& progressSink
) const {
	ProgressMerger progressMerger(progressSink);
	ProgressSink& audioProgressSink =
		progressMerger.addSource("audio preparation", 0.1);
	ProgressSink& vadProgressSink =
		progressMerger.addSource("voice activity detection", 0.05);
	ProgressSink& whisperProgressSink =
		progressMerger.addSource("Whisper inference", 0.65);
	ProgressSink& mappingProgressSink =
		progressMerger.addSource("phone mapping", 0.2);

	// Resolve model path
	path resolvedModelPath = resolveModelPath();
	if (!std::filesystem::exists(resolvedModelPath)) {
		throw std::runtime_error(
			"Whisper model not found at: " + resolvedModelPath.string()
			+ "\nDownload a model from https://huggingface.co/ggerganov/whisper.cpp"
			+ " and place it at the above path, or specify --whisperModel.");
	}

	// Prepare audio: remove DC offset and convert to float32 at 16kHz
	logging::info("Preparing audio for Whisper...");
	auto audioClip = inputAudioClip.clone() | removeDcOffset();
	vector<float> audioSamples = audioClipToFloat32(*audioClip);
	audioProgressSink.reportProgress(1.0);

	// Run voice activity detection to identify actual speech regions.
	// This prevents Whisper from placing words in silence.
	logging::info("Detecting voice activity...");
	JoiningBoundedTimeline<void> voiceActivity = detectVoiceActivity(
		*audioClip, vadProgressSink);

	// Run Whisper inference
	vector<RecognizedWord> words = recognizeWords(
		audioSamples, resolvedModelPath, dialog, maxThreadCount, whisperProgressSink);

	// Load CMU dictionary for word-to-phone mapping
	logging::info("Mapping words to phones...");
	path dictPath = getBinDirectory() / "res" / "sphinx" / "cmudict-en-us.dict";
	CmuDictionary dictionary(dictPath);

	// Create result timeline
	const auto audioRange = inputAudioClip.getTruncatedRange();
	BoundedTimeline<Phone> result(audioRange);

	int wordsProcessed = 0;
	for (const auto& word : words) {
		// Clamp word times to audio range
		centiseconds start = std::max(word.start, audioRange.getStart());
		centiseconds end = std::min(word.end, audioRange.getEnd());
		if (start >= end) continue;

		// Skip words that fall entirely outside voice-active regions
		bool inVoiceRegion = false;
		for (const auto& activity : voiceActivity) {
			// Check if the word's midpoint falls within a voice-active region
			centiseconds wordMiddle = start + (end - start) / 2;
			if (wordMiddle >= activity.getStart() && wordMiddle < activity.getEnd()) {
				inVoiceRegion = true;
				break;
			}
		}
		if (!inVoiceRegion) {
			logging::debugFormat("Skipping word '{}' at {}-{} (outside voice activity)",
				word.text, start.count(), end.count());
			++wordsProcessed;
			continue;
		}

		// Look up phones in CMU dictionary
		vector<Phone> phones = dictionary.lookup(word.text);

		// Fall back to G2P if not in dictionary
		if (phones.empty()) {
			phones = wordToPhones(word.text);
			if (!phones.empty()) {
				logging::debugFormat("G2P fallback for '{}': {} phones",
					word.text, phones.size());
			}
		}

		// If we still have no phones, mark as noise
		if (phones.empty()) {
			result.set(start, end, Phone::Noise);
		} else {
			distributePhones(phones, start, end, result);
		}

		++wordsProcessed;
		mappingProgressSink.reportProgress(
			static_cast<double>(wordsProcessed) / words.size());
	}

	mappingProgressSink.reportProgress(1.0);

	logging::info("Whisper recognition complete.");
	return result;
}
