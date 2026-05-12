#pragma once

#include "Recognizer.h"
#include "pocketSphinxTools.h"

class PocketSphinxRecognizer : public Recognizer {
public:
	PocketSphinxRecognizer() = default;
	explicit PocketSphinxRecognizer(BlendshapeVadParams blendshapeVad)
		: blendshapeVad_(std::move(blendshapeVad))
	{}

	BoundedTimeline<Phone> recognizePhones(
		const AudioClip& inputAudioClip,
		boost::optional<std::string> dialog,
		int maxThreadCount,
		ProgressSink& progressSink
	) const override;

private:
	BlendshapeVadParams blendshapeVad_;
};
