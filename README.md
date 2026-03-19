# Rhubarb Lip Sync

> **Note:** This is a fork of [DanielSWolf/rhubarb-lip-sync](https://github.com/DanielSWolf/rhubarb-lip-sync). Changes from the original include:
>
> - **Python bindings** -- use Rhubarb as a library via `pip install` and `import rhubarb`
> - **Four new extended mouth shapes** (I, J, K, L) for more detailed animation (wide smile, tongue-between-teeth, pursed lips, R sound)
> - **Target framerate support** (`--framerate`) -- snaps shape transitions to frame boundaries for cleaner animation at any fps
> - **Improved animation accuracy** -- consonant-vowel coarticulation, per-diphthong timing, expanded tween coverage, better static segment breaking, and expanded pronunciation fixes
> - Removed third-party integrations (Adobe After Effects, Moho, Spine, Vegas Pro) to simplify the codebase

---

Rhubarb Lip Sync allows you to quickly create 2D mouth animation from voice recordings. It analyzes your audio files, recognizes what is being said, then automatically generates lip sync information. You can use it for animating speech in computer games, animated cartoons, or any similar project.

You can use Rhubarb Lip Sync as a **Python library** or through its **command line interface** (**CLI**) to generate files in various [output formats](#output-formats) ([TSV](#tab-separated-values-tsv)/[XML](#xml-format-xml)/[JSON](#json-format-json)).

## Python usage

Install with pip (requires a C++ compiler and Boost headers):

```bash
pip install .
```

Then use it in Python:

```python
import rhubarb

cues = rhubarb.animate(
    "my-recording.wav",
    dialog="Optional dialog text",
    extended_shapes="GHIJKLX",  # enable all shapes
    framerate=12,               # snap to 12 fps frame boundaries
)

for cue in cues:
    print(f"{cue.start:.2f} - {cue.end:.2f}: {cue.shape}")
```

### Using Whisper (recommended for best accuracy)

The Whisper recognizer uses OpenAI's Whisper speech recognition model for more accurate word detection. Just pass a model name and it will be downloaded automatically on first use (~75MB for tiny, cached in `~/.cache/rhubarb/models/`):

```python
import rhubarb

cues = rhubarb.animate(
    "my-recording.wav",
    recognizer="whisper",
    whisper_model="tiny",           # auto-downloads on first use
    extended_shapes="GHIJKLX",
    framerate=12,
)
```

You can also provide an absolute path to a model file you've already downloaded:

```python
cues = rhubarb.animate(
    "my-recording.wav",
    recognizer="whisper",
    whisper_model="/path/to/ggml-tiny.bin",
)
```

Available model sizes: `"tiny"` (~75MB), `"base"` (~142MB), `"small"` (~466MB). Larger models are more accurate but slower.

### Parameters

- `input_file` -- path to a WAVE (.wav) or Ogg Vorbis (.ogg) file
- `dialog` -- optional dialog text to improve recognition accuracy
- `recognizer` -- `"pocketSphinx"` (English, default), `"phonetic"` (any language), or `"whisper"` (English, best accuracy)
- `extended_shapes` -- which extended shapes to use (default `"GHX"`, use `"GHIJKLX"` for all)
- `threads` -- max worker threads (default `0` = all CPU cores)
- `framerate` -- target animation frame rate in fps (default `0` = no frame snapping). When set, shape transitions are snapped to frame boundaries and minimum shape durations are adjusted so no shape is shorter than one frame.
- `whisper_model` -- Whisper model name (`"tiny"`, `"base"`, `"small"`) to auto-download, or a file path to a GGML model. Only used with `recognizer="whisper"`.

### Disabling Whisper at build time

To skip building Whisper support (faster compile, smaller binary):

```bash
pip install . --config-settings='cmake.args=-DRHUBARB_BUILD_WHISPER=OFF'
```

## Mouth shapes

Rhubarb Lip Sync can use between six and thirteen different mouth positions. The first six mouth shapes (A-F) are the *basic mouth shapes* and the absolute minimum you have to draw for your character. These six mouth shapes were invented at the Hanna-Barbera studios for shows such as Scooby-Doo and The Flintstones. Since then, they have evolved into a *de-facto* standard for 2D animation, and have been widely used by studios like Disney and Warner Bros.

In addition to the six basic mouth shapes, there are seven *extended mouth shapes*: G, H, I, J, K, L, and X. These are optional. You may choose to draw all of them, pick just a few, or leave them out entirely.

| Shape | Image | Description |
|:---:|:---:|:---|
| **A** | ![](img/lisa-A.png) | Closed mouth for the "P", "B", and "M" sounds. This is almost identical to the X shape, but there is ever-so-slight pressure between the lips. |
| **B** | ![](img/lisa-B.png) | Slightly open mouth with clenched teeth. This mouth shape is used for most consonants ("K", "S", "T", etc.). It's also used for some vowels such as the "EE" sound in b**ee**. |
| **C** | ![](img/lisa-C.png) | Open mouth. This mouth shape is used for vowels like "EH" as in m**e**n and "AE" as in b**a**t. It's also used for some consonants, depending on context. This shape is also used as an in-between when animating from A or B to D. |
| **D** | ![](img/lisa-D.png) | Wide open mouth. This mouth shape is used for vowels like "AA" as in f**a**ther. |
| **E** | ![](img/lisa-E.png) | Slightly rounded mouth. This mouth shape is used for vowels like "AO" as in **o**ff and "ER" as in b**ir**d. This shape is also used as an in-between when animating from C or D to F. |
| **F** | ![](img/lisa-F.png) | Puckered lips. This mouth shape is used for "UW" as in y**ou**, "OW" as in sh**ow**, and "W" as in **w**ay. |
| **G** | ![](img/lisa-G.png) | Upper teeth touching the lower lip for "F" as in **f**or and "V" as in **v**ery. *Optional. Falls back to A.* |
| **H** | ![](img/lisa-H.png) | Tongue raised behind the upper teeth for long "L" sounds. The mouth should be at least as far open as C, but not quite as far as D. *Optional. Falls back to C.* |
| **I** | | Wide smile. Used for the "EE" sound as in b**ee** and sh**e**, as well as the ending of diphthongs like s**ay**, m**y**, and b**oy**. Differs from B by having the lips pulled back into a wide smile. *Optional. Falls back to B.* |
| **J** | | Tongue between teeth. Used for "TH" as in **th**ink and "DH" as in **th**at, where the tongue tip is visible between the upper and lower teeth. *Optional. Falls back to B.* |
| **K** | | Pursed, protruded lips. Used for "SH" as in **sh**ow, "ZH" as in mea**s**ure, "CH" as in **ch**air, and "JH" as in **j**ust. The lips are pushed forward and slightly rounded, distinct from the tighter pucker of F. *Optional. Falls back to F.* |
| **L** | | Bunched tongue for the "R" sound as in **r**un and b**ir**d. The lips are slightly rounded and the tongue is bunched or retroflex. Visually similar to E but with more lip tension and a narrower opening. *Optional. Falls back to E.* |
| **X** | ![](img/lisa-X.png) | Idle position. Used for pauses in speech. This should be the same mouth drawing you use when your character is walking around without talking. Almost identical to A, but with slightly less pressure between the lips. *Optional. Falls back to A.* |

## How to run Rhubarb Lip Sync

### General usage

Rhubarb Lip Sync is a command-line tool that is currently available for Windows, macOS, and Linux.

- Download the [latest release](https://github.com/DanielSWolf/rhubarb-lip-sync/releases) for your operating system and unpack the file anywhere on your computer.
- On the command-line, call `rhubarb`, passing it an audio file as argument and telling it where to create the output file. In its simplest form, this might look like this: `rhubarb -o output.txt my-recording.wav`. There are additional [command-line options](#command-line-options) you can specify in order to get better results.
- Rhubarb Lip Sync will analyze the sound file, animate it, and create an output file containing the animation. If an error occurs, it will instead print an error message to `stderr` and exit with a non-zero exit code.

### Command-line options

#### Basic command-line options

| Option | Description |
|:---|:---|
| *\<input file\>* | The audio file to be analyzed. This must be the last command-line argument. Supported file formats are WAVE (.wav) and Ogg Vorbis (.ogg). |
| `-r` *\<recognizer\>*, `--recognizer` *\<recognizer\>* | Specifies how Rhubarb Lip Sync recognizes speech within the recording. Options: `pocketSphinx` (English, default), `phonetic` (any language), `whisper` (English, best accuracy -- requires model file). For details, see [Recognizers](#recognizers). |
| `-f` *\<format\>*, `--exportFormat` *\<format\>* | The export format. Options: `tsv` (tab-separated values), `xml`, `json`. Default: `tsv` |
| `-d` *\<path\>*, `--dialogFile` *\<path\>* | Provide the dialog text to get more reliable results. Specify the path to a plain-text file (ASCII or UTF-8) containing the dialog. Rhubarb Lip Sync will prefer words and phrases from this file while still performing its own recognition. It is always a good idea to specify the dialog text. |
| `--extendedShapes` *\<string\>* | Which extended mouth shapes to use. For example, `GHIJKLX` for all, or `""` for basic shapes only. Default: `GHX` |
| `--framerate` *\<number\>* | Target animation frame rate in fps (e.g. `12`, `24`, `30`). When set, shape transitions are snapped to frame boundaries and minimum shape durations are adjusted to match. `0` means no frame snapping. Default: `0` |
| `-o`, `--output` *\<output file\>* | The output file path. If omitted, results are written to `stdout`. |
| `--version` | Displays version information and exits. |
| `-h`, `--help` | Displays usage information and exits. |

#### Advanced command-line options

| Option | Description |
|:---|:---|
| `-q`, `--quiet` | Suppresses all output to `stderr` except for errors. Can be combined with `--consoleLevel`. |
| `--machineReadable` | Formats all `stderr` output as structured JSON. Useful for integrating with other applications. See [Machine-readable status messages](#machine-readable-status-messages). |
| `--consoleLevel` *\<level\>* | Sets the log level for `stderr`. Options: `trace`, `debug`, `info`, `warning`, `error`, `fatal`. Default: `error` |
| `--logFile` *\<path\>* | Creates a log file with diagnostic information at the specified path. |
| `--logLevel` *\<level\>* | Sets the log level for the log file. Default: `debug` |
| `--threads` *\<number\>* | Max worker threads. Default: number of CPU cores. |
| `--whisperModel` *\<path\>* | Path to a Whisper GGML model file (e.g. `ggml-tiny.bin`). Only used with `--recognizer whisper`. Download models from https://huggingface.co/ggerganov/whisper.cpp. |

## Recognizers

The first step in processing an audio file is determining what is being said. Rhubarb Lip Sync uses speech recognition to figure out what sound is being said at what point in time. You can choose between three recognizers:

### PocketSphinx

PocketSphinx is an open-source speech recognition library that generally gives good results. This is the default recognizer. The downside is that PocketSphinx only recognizes English dialog. So if your recordings are in a language other than English, this is not a good choice.

### Phonetic

Rhubarb Lip Sync also comes with a phonetic recognizer. *Phonetic* means that this recognizer won't try to understand entire (English) words and phrases. Instead, it will recognize individual sounds and syllables. The results are usually less precise than those from the PocketSphinx recognizer. The advantage is that this recognizer is language-independent. Use it if your recordings are not in English.

### Whisper

The Whisper recognizer uses [whisper.cpp](https://github.com/ggml-org/whisper.cpp), a C++ port of OpenAI's Whisper speech recognition model. It provides significantly more accurate word recognition than PocketSphinx, especially for natural conversational speech, varied accents, and noisy recordings. Words are then mapped to phonemes using the CMU pronunciation dictionary for lip sync animation.

Whisper requires a model file. The `tiny` model (~75MB) is recommended for a good balance of speed and accuracy. Download models from https://huggingface.co/ggerganov/whisper.cpp.

```bash
# Example usage
rhubarb -r whisper --whisperModel path/to/ggml-tiny.bin -o output.txt recording.wav
```

To build without Whisper support, configure CMake with `-DRHUBARB_BUILD_WHISPER=OFF`.

## Output formats

The output of Rhubarb Lip Sync is a file that tells you which mouth shape to display at what time within the recording. You can choose between three file formats -- TSV, XML, and JSON.

### Tab-separated values (`tsv`)

TSV is the simplest and most compact export format. Each line starts with a timestamp (in seconds), followed by a tab, followed by the name of the mouth shape. Example output for a recording of a person saying "Hi":

```
0.00	X
0.05	D
0.27	C
0.31	B
0.43	X
0.47	X
```

How to read it:

- At 0.00s, the mouth is closed (shape X).
- At 0.05s, the mouth opens wide (shape D) for the "HH" sound, anticipating the "AY" sound.
- At 0.27s, shape C is inserted as an in-between for a smooth transition from D to B.
- At 0.31s, clenched teeth (shape B) for the second half of the "AY" diphthong.
- At 0.43s, the dialog is finished and the mouth closes (shape X).
- The last line's timestamp is always the end of the recording with a closed mouth (X or A).

### XML format (`xml`)

```xml
<?xml version="1.0" encoding="utf-8"?>
<rhubarbResult>
  <metadata>
    <soundFile>C:\Users\Daniel\Desktop\av\hi\hi.wav</soundFile>
    <duration>0.47</duration>
  </metadata>
  <mouthCues>
    <mouthCue start="0.00" end="0.05">X</mouthCue>
    <mouthCue start="0.05" end="0.27">D</mouthCue>
    <mouthCue start="0.27" end="0.31">C</mouthCue>
    <mouthCue start="0.31" end="0.43">B</mouthCue>
    <mouthCue start="0.43" end="0.47">X</mouthCue>
  </mouthCues>
</rhubarbResult>
```

The file starts with a `metadata` block containing the full path of the original recording and its duration. Each `mouthCue` element indicates the start and end of a mouth shape. The end of each cue equals the start of the next one.

### JSON format (`json`)

```json
{
  "metadata": {
    "soundFile": "C:\\Users\\Daniel\\Desktop\\av\\hi\\hi.wav",
    "duration": 0.47
  },
  "mouthCues": [
    { "start": 0.00, "end": 0.05, "value": "X" },
    { "start": 0.05, "end": 0.27, "value": "D" },
    { "start": 0.27, "end": 0.31, "value": "C" },
    { "start": 0.31, "end": 0.43, "value": "B" },
    { "start": 0.43, "end": 0.47, "value": "X" }
  ]
}
```

JSON format is very similar to XML format. The choice mainly depends on which format your programming language supports best.

## Machine-readable status messages

Use the `--machineReadable` command-line option to enable machine-readable status messages. In this mode, each line printed to `stderr` will be a JSON object containing:

- `type`: One of `"start"`, `"progress"`, `"success"`, `"failure"`, or `"log"`.
- Event-specific data (e.g., `"value": 0.69` for progress events).
- `log`: A human-readable log message with severity information.

Example successful run:

```json
{ "type": "start", "file": "hi.wav", "log": { "level": "Info", "message": "Application startup. Input file: \"hi.wav\"." } }
{ "type": "progress", "value": 0.00, "log": { "level": "Trace", "message": "Progress: 0%" } }
{ "type": "progress", "value": 0.69, "log": { "level": "Trace", "message": "Progress: 68%" } }
{ "type": "progress", "value": 1.00, "log": { "level": "Trace", "message": "Progress: 100%" } }
{ "type": "success", "log": { "level": "Info", "message": "Application terminating normally." } }
```

Example failed run:

```json
{ "type": "start", "file": "no-such-file.wav", "log": { "level": "Info", "message": "Application startup. Input file: \"no-such-file.wav\"." } }
{ "type": "failure", "reason": "Error processing file \"no-such-file.wav\".\nCould not open sound file \"no-such-file.wav\".\nNo such file or directory", "log": { "level": "Fatal", "message": "..." } }
```

The output format [adheres to SemVer](#versioning-semver). Non-breaking changes that may occur at any time include: additional event types, additional properties, changes in formatting, and changes in log message wording.

## Versioning (SemVer)

Rhubarb Lip Sync uses [Semantic Versioning](http://semver.org/) for its command-line interface. As a rule of thumb, everything you can use through the CLI adheres to SemVer. Everything else (source code, etc.) does not.

## Building from source

You'll need:

- CMake 3.10+
- A C++ compiler that supports C++17 (Visual Studio 2019, Xcode 14, GCC 10, or Clang 12)
- A current version of Boost

Then:

1. Create an empty `build` directory within the repository
2. `cd build`
3. `cmake ..` (optionally pass generator/compiler flags; see `.github/workflows/ci.yml` for examples)
4. `cmake --build . --config Release`

To also build the Python bindings, add `-DRHUBARB_BUILD_PYTHON=ON` to the cmake configure step.

Whisper support is enabled by default. To disable it, add `-DRHUBARB_BUILD_WHISPER=OFF`.

## Changes from upstream

This fork includes the following changes on top of the original [DanielSWolf/rhubarb-lip-sync](https://github.com/DanielSWolf/rhubarb-lip-sync):

### New features

- **Python bindings** -- `pip install .` and `import rhubarb` to use Rhubarb as a Python library. Built with pybind11.
- **Four new extended mouth shapes** (I, J, K, L):
  - **I** -- Wide smile for "EE" and diphthong endings (s**ay**, m**y**, b**oy**)
  - **J** -- Tongue between teeth for "TH"/"DH" sounds
  - **K** -- Pursed, protruded lips for "SH", "CH", "ZH", "JH" sounds
  - **L** -- Bunched tongue for "R" sounds
- **Target framerate** (`--framerate` / `framerate=`) -- Snaps all shape transitions to frame boundaries and ensures no shape is shorter than one frame. For example, `--framerate 12` aligns all transitions to multiples of 1/12s.
- **Whisper speech recognition** (`-r whisper`) -- Optional recognizer using [whisper.cpp](https://github.com/ggml-org/whisper.cpp) for significantly more accurate word detection than PocketSphinx. Requires a model file download (~75MB for tiny). Includes a Python helper `rhubarb.download_whisper_model()` for easy model management.

### Animation accuracy improvements

- **Consonant-vowel coarticulation** -- Consonants with multiple shape options (T/D, K/G, N, NG, L, Y, etc.) now look ahead at the following vowel and prefer shapes that anticipate its lip posture. For example, a "T" before "OO" will favor a rounded shape rather than a flat one.
- **Per-diphthong timing** -- Diphthongs no longer use a universal 60/40 split. Each diphthong has its own ratio tuned to its articulation: EY (70/30), AY (55/45), OW (50/50), AW (55/45), OY (60/40).
- **Expanded tween coverage** -- Added 12 new shape-to-shape tween rules for high-effort transitions (F/K to/from D, F/K to A/X, I to/from F/K) that previously had jarring jumps with no intermediate shape.
- **Better static segment breaking** -- Long stretches of a single mouth shape are now broken up for shapes C, D, E, F, and I, not just B as before.
- **Expanded pronunciation fixes** -- Added preferred pronunciation overrides for 13 common function words (a, the, for, or, was, been, can, our, your, because) to produce more visually distinct animation.

### Other changes

- Removed third-party integrations (Adobe After Effects, Moho, Spine, Vegas Pro) to simplify the codebase.

## Credits

This is a fork of [Rhubarb Lip Sync](https://github.com/DanielSWolf/rhubarb-lip-sync) by Daniel S. Wolf. The original project provides the core speech recognition and animation engine. See the original repository for issues related to the upstream project.
