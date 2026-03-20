#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <string>
#include <vector>
#include <filesystem>
#include "lib/rhubarbLib.h"
#include "core/Shape.h"
#include "recognition/PocketSphinxRecognizer.h"
#include "recognition/PhoneticRecognizer.h"
#if RHUBARB_HAS_WHISPER
#include "recognition/WhisperRecognizer.h"
#include "recognition/WhisperPocketSphinxRecognizer.h"
#include "recognition/whisperTranscribe.h"
#include "audio/audioFileReading.h"
#endif
#include "animation/targetShapeSet.h"
#include "tools/progress.h"
#include "tools/parallel.h"
#include "tools/platformTools.h"

namespace py = pybind11;

using std::string;
using std::vector;
using std::unique_ptr;
using std::make_unique;
namespace fs = std::filesystem;

namespace {

struct MouthCue {
	double start;
	double end;
	string shape;
};

ShapeSet parseShapeSet(const string& extendedShapesString) {
	ShapeSet result(ShapeConverter::get().getBasicShapes());
	for (char ch : extendedShapesString) {
		Shape shape = ShapeConverter::get().parse(string(1, ch));
		result.insert(shape);
	}
	return result;
}

unique_ptr<Recognizer> createRecognizer(
	const string& recognizerName,
	const string& whisperModelPath = ""
) {
	if (recognizerName == "pocketSphinx") {
		return make_unique<PocketSphinxRecognizer>();
	}
	if (recognizerName == "phonetic") {
		return make_unique<PhoneticRecognizer>();
	}
#if RHUBARB_HAS_WHISPER
	if (recognizerName == "whisper") {
		return make_unique<WhisperRecognizer>(
			whisperModelPath.empty()
				? fs::path()
				: fs::u8path(whisperModelPath));
	}
	if (recognizerName == "whisperPocketSphinx") {
		return make_unique<WhisperPocketSphinxRecognizer>(
			whisperModelPath.empty()
				? fs::path()
				: fs::u8path(whisperModelPath));
	}
#endif
	throw std::invalid_argument(
		"Unknown recognizer: '" + recognizerName + "'."
		" Use 'pocketSphinx', 'phonetic'"
#if RHUBARB_HAS_WHISPER
		", 'whisper', or 'whisperPocketSphinx'"
#endif
		"."
	);
}

// Resolve the directory containing the native module (.so/.pyd)
fs::path getModuleDirectory() {
	py::module_ importlib = py::module_::import("importlib");
	py::module_ util = py::module_::import("importlib.util");
	py::object spec = util.attr("find_spec")("rhubarb._rhubarb");
	if (spec.is_none()) {
		spec = util.attr("find_spec")("_rhubarb");
	}
	if (spec.is_none()) {
		throw std::runtime_error("Cannot find _rhubarb module spec");
	}
	string origin = spec.attr("origin").cast<string>();
	return fs::path(origin).parent_path();
}

void initResourcePath() {
	static bool initialized = false;
	if (initialized) return;
	initialized = true;

	fs::path moduleDir = getModuleDirectory();
	// Resources are in res/ next to the .so, or in the package directory
	fs::path resPath = moduleDir / "res" / "sphinx" / "cmudict-en-us.dict";
	if (fs::exists(resPath)) {
		setBinDirectory(moduleDir);
	}
	// Otherwise, fall back to default getBinDirectory() behavior
}

#if RHUBARB_HAS_WHISPER
string transcribe(
	const string& inputFile,
	const string& whisperModel,
	int threads
) {
	initResourcePath();

	int actualThreads = threads > 0 ? threads : getProcessorCoreCount();
	fs::path modelPath = whisperModel.empty()
		? fs::path()
		: fs::u8path(whisperModel);

	// Resolve model path (same logic as WhisperRecognizer)
	if (modelPath.empty()) {
		modelPath = getBinDirectory() / "res" / "whisper" / "ggml-tiny.bin";
	}

	NullProgressSink progressSink;
	boost::optional<string> noDialog;

	string result;
	{
		py::gil_scoped_release release;
		// Read the audio file
		auto audioClip = createAudioFileClip(fs::u8path(inputFile));
		result = transcribeWithWhisper(*audioClip, modelPath, noDialog, actualThreads, progressSink);
	}
	return result;
}
#endif

vector<MouthCue> animate(
	const string& inputFile,
	const string& dialog,
	const string& recognizer,
	const string& extendedShapes,
	int threads,
	int framerate,
	const string& whisperModel
) {
	initResourcePath();

	int actualThreads = threads > 0 ? threads : getProcessorCoreCount();

	auto rec = createRecognizer(recognizer, whisperModel);
	ShapeSet targetShapeSet = parseShapeSet(extendedShapes);

	boost::optional<string> dialogOpt;
	if (!dialog.empty()) {
		dialogOpt = dialog;
	}

	NullProgressSink progressSink;

	JoiningContinuousTimeline<Shape> animation = [&] {
		py::gil_scoped_release release;
		return animateWaveFile(
			fs::u8path(inputFile),
			dialogOpt,
			*rec,
			targetShapeSet,
			actualThreads,
			progressSink,
			framerate);
	}();

	vector<MouthCue> result;
	for (const auto& timedShape : animation) {
		double startSecs = std::chrono::duration<double>(timedShape.getStart()).count();
		double endSecs = std::chrono::duration<double>(timedShape.getEnd()).count();

		std::ostringstream shapeName;
		shapeName << timedShape.getValue();

		result.push_back(MouthCue{startSecs, endSecs, shapeName.str()});
	}
	return result;
}

} // anonymous namespace

PYBIND11_MODULE(_rhubarb, m) {
	m.doc() = "Rhubarb Lip Sync - automatic lip sync from audio";

	py::class_<MouthCue>(m, "MouthCue")
		.def_readonly("start", &MouthCue::start,
			"Start time in seconds")
		.def_readonly("end", &MouthCue::end,
			"End time in seconds")
		.def_readonly("shape", &MouthCue::shape,
			"Mouth shape name (A-H, X)")
		.def("__repr__", [](const MouthCue& c) {
			return "MouthCue(start=" + std::to_string(c.start)
				+ ", end=" + std::to_string(c.end)
				+ ", shape='" + c.shape + "')";
		});

	m.def("animate", &animate,
		py::arg("input_file"),
		py::arg("dialog") = "",
		py::arg("recognizer") = "pocketSphinx",
		py::arg("extended_shapes") = "GHX",
		py::arg("threads") = 0,
		py::arg("framerate") = 0,
		py::arg("whisper_model") = "",
		R"doc(
		Analyze an audio file and generate lip sync mouth cues.

		Args:
			input_file: Path to a WAVE (.wav) or Ogg Vorbis (.ogg) audio file.
			dialog: Optional dialog text to improve recognition accuracy.
			recognizer: Speech recognizer to use: 'pocketSphinx' (English),
				'phonetic' (any language), 'whisper' (English, requires model file),
				or 'whisperPocketSphinx' (hybrid: Whisper transcription + PocketSphinx alignment).
			extended_shapes: Extended mouth shapes to use. Default 'GHX'. Use '' for basic shapes only (A-F).
			threads: Max worker threads. 0 means use all available CPU cores.
			framerate: Target animation frame rate in fps (e.g. 12, 24). Shape transitions
				are snapped to frame boundaries. 0 means no frame snapping (default).
			whisper_model: Path to a Whisper GGML model file. Only used with recognizer='whisper'.
				If empty, looks for the model in the default resource directory.

		Returns:
			List of MouthCue objects with start, end (seconds), and shape attributes.
		)doc"
	);

#if RHUBARB_HAS_WHISPER
	m.def("transcribe", &transcribe,
		py::arg("input_file"),
		py::arg("whisper_model") = "",
		py::arg("threads") = 0,
		R"doc(
		Transcribe an audio file using Whisper and return the text.

		Args:
			input_file: Path to a WAVE (.wav) or Ogg Vorbis (.ogg) audio file.
			whisper_model: Path to a Whisper GGML model file.
				If empty, looks for the model in the default resource directory.
			threads: Max worker threads. 0 means use all available CPU cores.

		Returns:
			Transcribed text as a string.
		)doc"
	);
#endif
}
