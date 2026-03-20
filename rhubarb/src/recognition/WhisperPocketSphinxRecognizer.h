#pragma once

#include "Recognizer.h"
#include <filesystem>

// Hybrid recognizer: uses Whisper to auto-transcribe the audio,
// then passes the transcript to PocketSphinx for phone-level forced alignment.
// Gives the best of both worlds: Whisper's superior word recognition +
// PocketSphinx's detailed phoneme alignment.
class WhisperPocketSphinxRecognizer : public Recognizer {
public:
	// modelPath: path to a Whisper GGML model file (e.g., ggml-tiny.bin).
	// If empty, uses the default model resolution logic.
	explicit WhisperPocketSphinxRecognizer(
		const std::filesystem::path& modelPath = std::filesystem::path());

	BoundedTimeline<Phone> recognizePhones(
		const AudioClip& inputAudioClip,
		boost::optional<std::string> dialog,
		int maxThreadCount,
		ProgressSink& progressSink
	) const override;

private:
	std::filesystem::path modelPath;
};
