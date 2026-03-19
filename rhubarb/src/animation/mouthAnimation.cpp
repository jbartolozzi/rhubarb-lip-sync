#include "mouthAnimation.h"
#include <cmath>
#include "time/timedLogging.h"
#include "ShapeRule.h"
#include "roughAnimation.h"
#include "pauseAnimation.h"
#include "tweening.h"
#include "timingOptimization.h"
#include "targetShapeSet.h"
#include "staticSegments.h"

// Snaps all shape transition times to the nearest frame boundary for the given framerate.
// This ensures that shape changes align with actual animation frames.
JoiningContinuousTimeline<Shape> quantizeToFrames(
	const JoiningContinuousTimeline<Shape>& animation,
	int framerate
) {
	if (framerate <= 0) return animation;

	const double frameDurationCs = 100.0 / framerate;

	// Quantize a time value to the nearest frame boundary.
	// Uses integer frame indices to avoid floating-point drift.
	auto quantize = [frameDurationCs](centiseconds time) -> centiseconds {
		const long long frameIndex = std::llround(time.count() / frameDurationCs);
		return centiseconds(static_cast<int>(std::llround(frameIndex * frameDurationCs)));
	};

	const TimeRange range = animation.getRange();
	const centiseconds quantizedStart = quantize(range.getStart());
	centiseconds quantizedEnd = quantize(range.getEnd());
	// Ensure the quantized range is non-empty
	if (quantizedEnd <= quantizedStart) {
		quantizedEnd = quantizedStart + centiseconds(
			static_cast<int>(std::ceil(frameDurationCs))
		);
	}
	JoiningContinuousTimeline<Shape> result(
		TimeRange(quantizedStart, quantizedEnd), Shape::X
	);

	Shape lastShape = Shape::X;
	for (const auto& timedShape : animation) {
		centiseconds start = quantize(timedShape.getStart());
		centiseconds end = quantize(timedShape.getEnd());
		// If quantization collapsed this shape, extend it to at least one frame
		if (start >= end) {
			end = start + centiseconds(static_cast<int>(std::ceil(frameDurationCs)));
		}
		// Clamp to the result range
		start = std::max(start, quantizedStart);
		end = std::min(end, quantizedEnd);
		if (start >= end) continue;
		result.set(start, end, timedShape.getValue());
		lastShape = timedShape.getValue();
	}

	return result;
}

JoiningContinuousTimeline<Shape> animate(
	const BoundedTimeline<Phone>& phones,
	const ShapeSet& targetShapeSet,
	int framerate
) {
	// Create timeline of shape rules
	ContinuousTimeline<ShapeRule> shapeRules = getShapeRules(phones);

	// Modify shape rules to only contain allowed shapes -- plus X, which is needed for pauses and
	// will be replaced later
	ShapeSet targetShapeSetPlusX = targetShapeSet;
	targetShapeSetPlusX.insert(Shape::X);
	shapeRules = convertToTargetShapeSet(shapeRules, targetShapeSetPlusX);

	// Animate in multiple steps
	const auto performMainAnimationSteps = [&targetShapeSet, framerate](const auto& shapeRules) {
		JoiningContinuousTimeline<Shape> animation = animateRough(shapeRules);
		animation = optimizeTiming(animation, framerate);
		animation = animatePauses(animation);
		animation = insertTweens(animation);
		animation = convertToTargetShapeSet(animation, targetShapeSet);
		return animation;
	};
	JoiningContinuousTimeline<Shape> result =
		avoidStaticSegments(shapeRules, performMainAnimationSteps);

	// Snap shape transitions to frame boundaries
	result = quantizeToFrames(result, framerate);

	for (const auto& timedShape : result) {
		logTimedEvent("shape", timedShape);
	}

	return result;
}
