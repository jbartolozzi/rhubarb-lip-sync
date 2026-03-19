#include "rhubarbLib.h"
#include "core/Phone.h"
#include "tools/textFiles.h"
#include "animation/mouthAnimation.h"
#include "audio/audioFileReading.h"

using boost::optional;
using std::string;
using std::filesystem::path;

JoiningContinuousTimeline<Shape> animateAudioClip(
	const AudioClip& audioClip,
	const optional<string>& dialog,
	const Recognizer& recognizer,
	const ShapeSet& targetShapeSet,
	int maxThreadCount,
	ProgressSink& progressSink,
	int framerate)
{
	const BoundedTimeline<Phone> phones =
		recognizer.recognizePhones(audioClip, dialog, maxThreadCount, progressSink);
	JoiningContinuousTimeline<Shape> result = animate(phones, targetShapeSet, framerate);
	return result;
}

JoiningContinuousTimeline<Shape> animateWaveFile(
	path filePath,
	const optional<string>& dialog,
	const Recognizer& recognizer,
	const ShapeSet& targetShapeSet,
	int maxThreadCount,
	ProgressSink& progressSink,
	int framerate)
{
	const auto audioClip = createAudioFileClip(filePath);
	return animateAudioClip(*audioClip, dialog, recognizer, targetShapeSet, maxThreadCount, progressSink, framerate);
}
