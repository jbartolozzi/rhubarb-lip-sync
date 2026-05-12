#include "BlendshapeTimeline.h"

#include "time/centiseconds.h"

#include <algorithm>
#include <cmath>
#include <utility>
#include <vector>

using std::size_t;
using std::vector;
using std::pair;

BlendshapeTimeline::BlendshapeTimeline(
    const double* timestamps,
    const float* values,
    size_t frameCount,
    size_t valuesPerFrame)
    : timestamps_(timestamps)
    , values_(values)
    , frameCount_(frameCount)
    , valuesPerFrame_(valuesPerFrame)
{}

namespace {

// Converts a (seconds, monotonic) double to rhubarb's centiseconds.
centiseconds toCentiseconds(double seconds) {
    return centiseconds(static_cast<int>(std::round(seconds * 100.0)));
}

}

JoiningBoundedTimeline<void> BlendshapeTimeline::mouthMotionMask(
    TimeRange bounds,
    float jawOpenThreshold,
    int minSilenceMs,
    size_t jawOpenIndex) const
{
    JoiningBoundedTimeline<void> result(bounds);
    if (frameCount_ == 0 || jawOpenIndex >= valuesPerFrame_) {
        return result;
    }

    // Walk the track and emit (startSec, endSec) intervals for runs of
    // frames whose jawOpen exceeds threshold. The first/last frame of a
    // run defines the interval boundaries — we don't extrapolate beyond
    // observed timestamps, so the mask stays conservatively inside what
    // the ARKit data actually saw.
    vector<pair<double, double>> motionIntervals;
    motionIntervals.reserve(frameCount_ / 4);

    bool inMotion = false;
    double runStart = 0.0;
    double prevTime = timestamps_[0];
    for (size_t i = 0; i < frameCount_; ++i) {
        const double t = timestamps_[i];
        const bool isMotion = weight(i, jawOpenIndex) > jawOpenThreshold;
        if (isMotion && !inMotion) {
            inMotion = true;
            runStart = t;
        } else if (!isMotion && inMotion) {
            inMotion = false;
            motionIntervals.emplace_back(runStart, prevTime);
        }
        prevTime = t;
    }
    if (inMotion) {
        motionIntervals.emplace_back(runStart, prevTime);
    }

    // Merge adjacent intervals that are separated by less than the
    // minimum silence window. Without this, every short consonant-driven
    // lip closure would chop a single utterance into many slivers.
    const double minSilenceSec =
        minSilenceMs > 0 ? minSilenceMs / 1000.0 : 0.0;
    vector<pair<double, double>> merged;
    merged.reserve(motionIntervals.size());
    for (auto& interval : motionIntervals) {
        if (!merged.empty()
            && interval.first - merged.back().second < minSilenceSec)
        {
            merged.back().second = interval.second;
        } else {
            merged.push_back(interval);
        }
    }

    for (const auto& [startSec, endSec] : merged) {
        result.set(toCentiseconds(startSec), toCentiseconds(endSec));
    }
    return result;
}
