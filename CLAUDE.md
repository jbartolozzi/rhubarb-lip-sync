# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Rhubarb Lip Sync is a cross-platform C++17 tool that analyzes voice recordings to automatically generate lip-sync animation data. It recognizes speech and maps phonemes to mouth shapes (A–L plus X — 13 total: 6 basic + 7 extended) for use in animation software.

This repository is a **fork** of [DanielSWolf/rhubarb-lip-sync](https://github.com/DanielSWolf/rhubarb-lip-sync) with: Python bindings, Whisper recognizer support, four additional extended mouth shapes (I/J/K/L), target-framerate snapping, animation-accuracy improvements, and removal of upstream third-party DCC integrations (After Effects, Moho, Spine, Vegas Pro). See `README.md` for the full list of fork-specific changes.

## Build Commands

```bash
# Configure (from repo root)
mkdir build && cd build
cmake ..

# Build
cmake --build . --config Release

# Run tests
./build/rhubarb/runTests        # macOS/Linux
build\rhubarb\runTests.exe      # Windows

# Create distributable package
cmake --build . --config Release --target package

# Platform packaging scripts
./package-osx.sh                # macOS (uses Xcode)
package-win.bat                 # Windows (uses VS)
```

Google Test supports `--gtest_filter=` for running individual tests (e.g. `./runTests --gtest_filter=TimelineTests.*`).

```bash
# Build Python bindings
cmake -B build -DRHUBARB_BUILD_PYTHON=ON
cmake --build build --target _rhubarb --config Release

# Or install via pip (requires Boost headers available)
pip install .

# Disable Whisper support (faster compile, smaller binary)
cmake -B build -DRHUBARB_BUILD_WHISPER=OFF
# or, for pip install:
pip install . --config-settings='cmake.args=-DRHUBARB_BUILD_WHISPER=OFF'

# Build macOS XCFramework for Swift consumption (arm64 + x86_64)
./package-xcframework.sh
```

CMake options of note: `RHUBARB_BUILD_PYTHON` (default OFF), `RHUBARB_BUILD_WHISPER` (default ON), `RHUBARB_BUILD_C_API` (default OFF — produces the `rhubarb-c` static lib that the XCFramework wraps).

## Architecture

### Core Pipeline

Audio file → `animateAudioClip()` (in `rhubarb/src/lib/rhubarbLib.cpp`) → Recognition → Phone timeline → Animation rules → Mouth shape timeline → Export

### Key Modules (under `rhubarb/src/`)

- **lib/**: High-level API entry points (`animateAudioClip`, `animateWaveFile`)
- **recognition/**: Speech recognition backends, all implementing `Recognizer.h`:
  - `PocketSphinxRecognizer` — English, acoustic model (default)
  - `PhoneticRecognizer` — language-independent, energy-based
  - `WhisperRecognizer` — uses vendored whisper.cpp; words → phonemes via CMU dictionary (best accuracy, English only). Compiled in only when `RHUBARB_BUILD_WHISPER=ON`.
  - `WhisperPocketSphinxRecognizer` — uses Whisper to produce a transcript that is then fed to PocketSphinx as dialog text (auto-generates dialog when none was supplied)
- **core/**: Fundamental types — `Shape` enum (13 mouth shapes: A–L plus X), `Phone` enum (phoneme set)
- **animation/**: Shape selection rules (`animationRules`, `ShapeRule`), `mouthAnimation`, `tweening`, `timingOptimization`, `staticSegments`, `pauseAnimation`. Includes consonant–vowel coarticulation and per-diphthong timing logic.
- **audio/**: Audio I/O (WAVE, OGG Vorbis), sample rate conversion, voice activity detection (uses WebRTC VAD)
- **time/**: Timeline data structures (`Timeline<T>`, `BoundedTimeline<T>`, `ContinuousTimeline<T>`) — central abstraction for all temporal data
- **exporters/**: Output formats — TSV, XML, JSON
- **rhubarb/**: CLI entry point (`main.cpp`) — argument parsing, recognizer/exporter selection, logging setup
- **python/**: pybind11 bindings (built with `-DRHUBARB_BUILD_PYTHON=ON`). The C++ extension is `_rhubarb`; the user-facing Python package lives at `python/rhubarb/__init__.py` and exposes `animate()` plus `download_whisper_model()`.
- **capi/**: C ABI wrapper for non-Python consumers (Swift via XCFramework). Header `Rhubarb.h`, impl `Rhubarb.cpp`, plus `module.modulemap`. Built as the static lib `rhubarb-c` when `-DRHUBARB_BUILD_C_API=ON`. Exposes `rhubarb_animate`, `rhubarb_set_resource_directory`, `rhubarb_last_error`, etc. — translates exceptions to `RhubarbStatus` and uses thread-local storage for error messages. Whisper is intentionally not exposed through this surface (callers integrate whisper.cpp's own xcframework if they need it).

### Animation parameters surfaced through both CLI and Python

These options exist as both CLI flags and `animate()` kwargs and are wired through the same code paths — keep them in sync when adding new ones:

- `recognizer` / `-r` — `pocketSphinx` | `phonetic` | `whisper`
- `extended_shapes` / `--extendedShapes` — subset string of `GHIJKLX` (default `GHX`); shapes not in the set fall back per the table in `README.md`
- `framerate` / `--framerate` — when non-zero, snaps shape transitions to frame boundaries and adjusts minimum shape durations so no shape is shorter than one frame
- `whisper_model` / `--whisperModel` — model name (`tiny`/`base`/`small`, auto-downloaded to `~/.cache/rhubarb/models/`) or absolute path to a GGML model

### Swift / XCFramework path

`package-xcframework.sh` orchestrates: per-arch CMake configure (`-DCMAKE_OSX_ARCHITECTURES=arm64` and `=x86_64`) with `RHUBARB_BUILD_C_API=ON RHUBARB_BUILD_WHISPER=OFF` → builds `rhubarb-c` → flattens every `*.a` in the build tree (excluding gtest) into one fat per-arch archive via `libtool -static` → `lipo -create` to produce a single universal macOS slice → `xcodebuild -create-xcframework` → copies the resulting `Rhubarb.xcframework` and the `res/sphinx/` tree into `swift/` for the Swift Package to pick up. `swift/Package.swift` exposes a `binaryTarget` named `RhubarbC` (the xcframework — module name must stay `RhubarbC` in [rhubarb/src/capi/module.modulemap](rhubarb/src/capi/module.modulemap) to avoid colliding with the Swift overlay target name) and a Swift overlay (`swift/Sources/Rhubarb/Rhubarb.swift`) that maps the C ABI to typed Swift (`Shape`, `MouthCue`, `AnimationOptions`, `RhubarbError`).

Consumer-facing integration instructions (SwiftPM and drag-in paths, resource bundling, sandboxing) live in [swift/INTEGRATION.md](swift/INTEGRATION.md). Update that file when the public Swift API changes.

### External Libraries (vendored in `rhubarb/lib/`)

All dependencies are vendored (PocketSphinx, Sphinxbase, Flite, WebRTC, Vorbis/Ogg, whisper.cpp, etc.) — no external package manager needed. Google Test is fetched via CMake FetchContent. Whisper (`whisper.cpp`) is conditionally compiled based on `RHUBARB_BUILD_WHISPER`.

## Build System Details

- **CMake 3.24+** with platform-specific configuration
- Version info defined in `appInfo.cmake`
- Requires: Boost 1.54+ (headers cached in CI at 1.86.0)
- Targets: Windows (VS 2019+, static CRT), macOS 10.15+ (Xcode 14+), Linux (GCC 10+/Clang 12+)
- MSVC uses `/utf-8` for Unicode support

## CI/CD

GitHub Actions (`.github/workflows/ci.yml`) builds on all three platforms, runs tests, and auto-creates GitHub release drafts when a `v*` tag is pushed (extracts notes from `CHANGELOG.md`).

## Tests

Unit tests are in `rhubarb/tests/` with test resources in `rhubarb/tests/resources/`. Test suites cover: string tools, Timeline types, tokenization, grapheme-to-phoneme conversion, Lazy evaluation, and WAVE file reading. Framework: Google Test + Google Mock.
