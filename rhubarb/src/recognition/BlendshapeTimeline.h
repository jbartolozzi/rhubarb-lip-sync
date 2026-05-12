#pragma once

#include "time/BoundedTimeline.h"
#include "time/TimeRange.h"
#include <cstddef>

// Non-owning view over an ARKit-style blendshape track passed through
// the C API. The underlying arrays are owned by the caller and must
// outlive every BlendshapeTimeline that references them — typically the
// caller keeps them alive for the duration of one rhubarb_animate call.
//
// `values` is laid out row-major: the j-th weight for frame i lives at
// values[i * valuesPerFrame + j]. `timestamps[i]` is the time (seconds,
// monotonic) of frame i. valuesPerFrame is the per-frame blendshape
// count, typically 52 for an unfiltered ARKit set.
class BlendshapeTimeline {
public:
    // ARKit canonical index of `jawOpen`. Matches the index used by the
    // Mocap Studio Swift side (BlendshapeName.allCases enumeration).
    static constexpr std::size_t arkitJawOpenIndex = 25;

    BlendshapeTimeline(
        const double* timestamps,
        const float* values,
        std::size_t frameCount,
        std::size_t valuesPerFrame);

    bool empty() const { return frameCount_ == 0; }
    std::size_t size() const { return frameCount_; }
    std::size_t valuesPerFrame() const { return valuesPerFrame_; }

    double timestamp(std::size_t frame) const { return timestamps_[frame]; }
    float weight(std::size_t frame, std::size_t blendshapeIndex) const {
        return values_[frame * valuesPerFrame_ + blendshapeIndex];
    }

    // Builds a timeline of the regions where the mouth is moving,
    // measured by `weight(frame, jawOpenIndex) > threshold`. Consecutive
    // motion runs separated by a quiet gap shorter than
    // `minSilenceMs` are merged so brief lip closures inside a syllable
    // (e.g. /p/, /b/, /m/) don't fragment an utterance. The returned
    // timeline is clipped to `bounds`.
    //
    // If the underlying data is empty or jawOpenIndex is out of range,
    // returns an empty timeline. Callers should treat that as "no
    // visual evidence" — typically by NOT intersecting and falling back
    // to the audio-only VAD.
    JoiningBoundedTimeline<void> mouthMotionMask(
        TimeRange bounds,
        float jawOpenThreshold,
        int minSilenceMs,
        std::size_t jawOpenIndex = arkitJawOpenIndex) const;

private:
    const double* timestamps_;
    const float* values_;
    std::size_t frameCount_;
    std::size_t valuesPerFrame_;
};
