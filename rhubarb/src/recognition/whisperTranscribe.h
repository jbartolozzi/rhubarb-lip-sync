#pragma once

#include <string>
#include <filesystem>
#include "audio/AudioClip.h"
#include "tools/progress.h"
#include <boost/optional.hpp>

// Run Whisper on an audio clip and return the plain-text transcript.
// This is used by both WhisperRecognizer (for word timestamps) and
// WhisperPocketSphinxRecognizer (to auto-generate dialog for PocketSphinx).
std::string transcribeWithWhisper(
	const AudioClip& audioClip,
	const std::filesystem::path& modelPath,
	const boost::optional<std::string>& dialog,
	int maxThreadCount,
	ProgressSink& progressSink
);
