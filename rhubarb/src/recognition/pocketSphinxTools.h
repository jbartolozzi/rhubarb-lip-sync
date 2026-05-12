#pragma once

#include "BlendshapeTimeline.h"
#include "time/BoundedTimeline.h"
#include "core/Phone.h"
#include "audio/AudioClip.h"
#include "tools/progress.h"
#include <filesystem>
#include <memory>

extern "C" {
#include <pocketsphinx.h>
}

typedef std::function<lambda_unique_ptr<ps_decoder_t>(
	boost::optional<std::string> dialog
)> decoderFactory;

typedef std::function<Timeline<Phone>(
	const AudioClip& audioClip,
	TimeRange utteranceTimeRange,
	ps_decoder_t& decoder,
	ProgressSink& utteranceProgressSink
)> utteranceToPhonesFunction;

// Parameters for the optional blendshape-driven VAD intersection. When
// `track` is non-null, recognizePhones intersects rhubarb's audio VAD
// output with `track->mouthMotionMask(...)` so utterances are kept only
// where both audio AND visual evidence agree on speech. With a null
// track these parameters are ignored and behavior matches upstream.
struct BlendshapeVadParams {
	std::shared_ptr<const BlendshapeTimeline> track;
	float jawOpenThreshold = 0.05f;
	int minSilenceMs = 200;
};

BoundedTimeline<Phone> recognizePhones(
	const AudioClip& inputAudioClip,
	boost::optional<std::string> dialog,
	decoderFactory createDecoder,
	utteranceToPhonesFunction utteranceToPhones,
	int maxThreadCount,
	ProgressSink& progressSink,
	const BlendshapeVadParams& blendshapeVad = {}
);

constexpr int sphinxSampleRate = 16000;

const std::filesystem::path& getSphinxModelDirectory();

JoiningTimeline<void> getNoiseSounds(TimeRange utteranceTimeRange, const Timeline<Phone>& phones);

BoundedTimeline<std::string> recognizeWords(
	const std::vector<int16_t>& audioBuffer,
	ps_decoder_t& decoder
);
