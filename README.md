# Rhubarb Lip Sync

> **Note:** This is a fork of [DanielSWolf/rhubarb-lip-sync](https://github.com/DanielSWolf/rhubarb-lip-sync). Changes from the original include:
>
> - **Python bindings** -- use Rhubarb as a library via `pip install` and `import rhubarb`
> - **Four new extended mouth shapes** (I, J, K, L) for more detailed animation (wide smile, tongue-between-teeth, pursed lips, R sound)
> - Removed third-party integrations (Adobe After Effects, Moho, Spine, Vegas Pro) to simplify the codebase

---

<p align="center"><img src="img/logo.png" alt="Rhubarb Lip Sync logo"></p>

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
)

for cue in cues:
    print(f"{cue.start:.2f} - {cue.end:.2f}: {cue.shape}")
```

Parameters:

- `input_file` -- path to a WAVE (.wav) or Ogg Vorbis (.ogg) file
- `dialog` -- optional dialog text to improve recognition accuracy
- `recognizer` -- `"pocketSphinx"` (English, default) or `"phonetic"` (any language)
- `extended_shapes` -- which extended shapes to use (default `"GHX"`, use `"GHIJKLX"` for all)
- `threads` -- max worker threads (default `0` = all CPU cores)

## Demo video

Click the image for a demo video.

[![Demo video](http://img.youtube.com/vi/zzdPSFJRlEo/0.jpg)](https://www.youtube.com/watch?v=zzdPSFJRlEo)

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
| `-r` *\<recognizer\>*, `--recognizer` *\<recognizer\>* | Specifies how Rhubarb Lip Sync recognizes speech within the recording. Options: `pocketSphinx` (use for English recordings), `phonetic` (use for non-English recordings). For details, see [Recognizers](#recognizers). Default: `pocketSphinx` |
| `-f` *\<format\>*, `--exportFormat` *\<format\>* | The export format. Options: `tsv` (tab-separated values), `xml`, `json`. Default: `tsv` |
| `-d` *\<path\>*, `--dialogFile` *\<path\>* | Provide the dialog text to get more reliable results. Specify the path to a plain-text file (ASCII or UTF-8) containing the dialog. Rhubarb Lip Sync will prefer words and phrases from this file while still performing its own recognition. It is always a good idea to specify the dialog text. |
| `--extendedShapes` *\<string\>* | Which extended mouth shapes to use. For example, `GHIJKLX` for all, or `""` for basic shapes only. Default: `GHX` |
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

## Recognizers

The first step in processing an audio file is determining what is being said. Rhubarb Lip Sync uses speech recognition to figure out what sound is being said at what point in time. You can choose between two recognizers:

### PocketSphinx

PocketSphinx is an open-source speech recognition library that generally gives good results. This is the default recognizer. The downside is that PocketSphinx only recognizes English dialog. So if your recordings are in a language other than English, this is not a good choice.

### Phonetic

Rhubarb Lip Sync also comes with a phonetic recognizer. *Phonetic* means that this recognizer won't try to understand entire (English) words and phrases. Instead, it will recognize individual sounds and syllables. The results are usually less precise than those from the PocketSphinx recognizer. The advantage is that this recognizer is language-independent. Use it if your recordings are not in English.

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

## Credits

This is a fork of [Rhubarb Lip Sync](https://github.com/DanielSWolf/rhubarb-lip-sync) by Daniel S. Wolf. The original project provides the core speech recognition and animation engine. See the original repository for issues related to the upstream project.
