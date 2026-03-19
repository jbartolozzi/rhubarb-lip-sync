#pragma once

#include "Recognizer.h"
#include <filesystem>

// Speech recognizer using whisper.cpp for word-level timestamps,
// then mapping words to phones via the CMU pronunciation dictionary.
// Gives more accurate word recognition than PocketSphinx while
// maintaining phone-level detail through dictionary lookup.
class WhisperRecognizer : public Recognizer {
public:
	// modelPath: path to a Whisper GGML model file (e.g., ggml-tiny.bin).
	// If empty, looks for the model in the default resource directory.
	explicit WhisperRecognizer(
		const std::filesystem::path& modelPath = std::filesystem::path());

	BoundedTimeline<Phone> recognizePhones(
		const AudioClip& inputAudioClip,
		boost::optional<std::string> dialog,
		int maxThreadCount,
		ProgressSink& progressSink
	) const override;

private:
	std::filesystem::path modelPath;

	std::filesystem::path resolveModelPath() const;
};
