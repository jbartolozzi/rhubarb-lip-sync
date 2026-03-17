# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Rhubarb Lip Sync is a cross-platform C++17 tool that analyzes voice recordings to automatically generate lip-sync animation data. It recognizes speech using two engines (PocketSphinx for English, Phonetic for other languages) and maps phonemes to mouth shapes (A-X) for use in animation software.

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
```

## Architecture

### Core Pipeline

Audio file → `animateAudioClip()` (in `rhubarb/src/lib/rhubarbLib.cpp`) → Recognition → Phone timeline → Animation rules → Mouth shape timeline → Export

### Key Modules (under `rhubarb/src/`)

- **lib/**: High-level API entry points (`animateAudioClip`, `animateWaveFile`)
- **recognition/**: Speech recognition backends — `PocketSphinxRecognizer` (English, uses acoustic model) and `PhoneticRecognizer` (language-independent, uses energy-based detection)
- **core/**: Fundamental types — `Shape` enum (9 mouth shapes A-X), `Phone` enum (phoneme set)
- **animation/**: Shape selection rules, mouth animation logic, tweening, and timing optimization
- **audio/**: Audio I/O (WAVE, OGG Vorbis), sample rate conversion, voice activity detection (uses WebRTC VAD)
- **time/**: Timeline data structures (`Timeline<T>`, `BoundedTimeline<T>`, `ContinuousTimeline<T>`) — central abstraction for all temporal data
- **exporters/**: Output formats — TSV, XML, JSON
- **rhubarb/**: CLI entry point (`main.cpp`) — argument parsing, recognizer/exporter selection, logging setup
- **python/**: pybind11 bindings exposing `animate()` to Python (built with `-DRHUBARB_BUILD_PYTHON=ON`)

### External Libraries (vendored in `rhubarb/lib/`)

All dependencies are vendored (PocketSphinx, Sphinxbase, Flite, WebRTC, Vorbis/Ogg, etc.) — no external package manager needed. Google Test is fetched via CMake FetchContent.

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
