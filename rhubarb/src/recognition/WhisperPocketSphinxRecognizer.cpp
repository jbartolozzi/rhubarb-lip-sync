#include "WhisperPocketSphinxRecognizer.h"
#include "PocketSphinxRecognizer.h"
#include "whisperTranscribe.h"
#include "logging/logging.h"
#include "tools/progress.h"
#include "tools/platformTools.h"

using std::string;
using std::filesystem::path;
using boost::optional;

WhisperPocketSphinxRecognizer::WhisperPocketSphinxRecognizer(
	const path& modelPath
) : modelPath(modelPath) {}

static path resolveModelPath(const path& modelPath) {
	if (!modelPath.empty()) {
		return modelPath;
	}
	return getBinDirectory() / "res" / "whisper" / "ggml-tiny.bin";
}

BoundedTimeline<Phone> WhisperPocketSphinxRecognizer::recognizePhones(
	const AudioClip& inputAudioClip,
	optional<string> dialog,
	int maxThreadCount,
	ProgressSink& progressSink
) const {
	// Split progress: 30% for Whisper transcription, 70% for PocketSphinx alignment
	ProgressMerger merger(progressSink);
	ProgressSink& whisperProgress = merger.addSource("Whisper transcription", 0.3);
	ProgressSink& pocketSphinxProgress = merger.addSource("PocketSphinx alignment", 0.7);

	// Phase 1: Use Whisper to auto-transcribe the audio
	path resolvedPath = resolveModelPath(modelPath);
	string transcript = transcribeWithWhisper(
		inputAudioClip, resolvedPath, dialog, maxThreadCount, whisperProgress);

	if (transcript.empty()) {
		logging::warn("Whisper produced empty transcript, falling back to PocketSphinx without dialog.");
	} else {
		logging::infoFormat("Using Whisper transcript as dialog for PocketSphinx: \"{}\"", transcript);
	}

	// Phase 2: Run PocketSphinx with the Whisper transcript as dialog
	optional<string> dialogForPocketSphinx = transcript.empty()
		? dialog  // Fall back to original dialog if Whisper failed
		: optional<string>(transcript);

	PocketSphinxRecognizer pocketSphinx;
	return pocketSphinx.recognizePhones(
		inputAudioClip, dialogForPocketSphinx, maxThreadCount, pocketSphinxProgress);
}
