# Rhubarb Swift Package

A Swift wrapper around the Rhubarb Lip Sync C ABI. Targets macOS arm64 + x86_64.

## Build

This package is **not buildable from a fresh checkout** — the `Rhubarb.xcframework` and PocketSphinx resources are produced by a CMake build, not committed.

From the repo root:

```sh
./package-xcframework.sh
```

That produces:

- `swift/Rhubarb.xcframework/`
- `swift/Sources/Rhubarb/Resources/res/sphinx/...` (copied into `Bundle.module`)

After that, `swift build` and `swift test` work from `swift/`.

## Usage

```swift
import Rhubarb

try Rhubarb.useBundledResources()      // or Rhubarb.setResourceDirectory(myURL)

var options = AnimationOptions()
options.recognizer     = .pocketSphinx
options.extendedShapes = "GHIJKLX"
options.framerate      = 24

let cues = try Rhubarb.animate(
    audioFile: URL(fileURLWithPath: "/path/to/recording.wav"),
    options: options
)

for cue in cues {
    print("\(cue.start)–\(cue.end): \(cue.shape)")
}
```

## Notes

- Whisper is **not** bundled. To use Whisper, link `whisper.cpp`'s own xcframework alongside this one.
- Resources (`res/sphinx/`) are loaded from `Bundle.module`. If you embed Rhubarb in an app and want to load resources from elsewhere (e.g. `Bundle.main`), call `Rhubarb.setResourceDirectory(_:)` with a URL whose `res/sphinx/cmudict-en-us.dict` exists.
- Errors are typed as `RhubarbError`. The C-side `rhubarb_last_error()` is captured at the throwing site.
