# Integrating Rhubarb into a Swift / macOS app

This guide covers using Rhubarb Lip Sync from a Swift application on macOS via the prebuilt `Rhubarb.xcframework`.

> Platforms: macOS 11+ (universal arm64 + x86_64). iOS is not supported by this build.

## 1. Build the framework

From the repository root:

```bash
./package-xcframework.sh
```

This produces:

- `swift/Rhubarb.xcframework` — universal static library
- `swift/Sources/Rhubarb/Resources/res/sphinx/...` — PocketSphinx model files used at runtime
- `build-xcframework/Rhubarb.xcframework` — same framework, standalone
- `build-xcframework/RhubarbResources/res/sphinx/...` — same resources, standalone

You only need to rerun this when Rhubarb's source changes. Whisper is not built into the framework; if you want Whisper, ship [whisper.cpp](https://github.com/ggerganov/whisper.cpp)'s own xcframework alongside Rhubarb's and call it from your app directly.

---

## 2. Pick an integration path

There are two ways to consume Rhubarb. Pick whichever fits your project.

### Path A — Swift Package Manager (recommended)

Best when your project is already a Swift package, or when you want Xcode to manage the binary and resources for you.

1. In Xcode: **File → Add Package Dependencies → Add Local…** and pick the `swift/` directory of this repo.
2. Add the `Rhubarb` library product to your app target.

That's it. The xcframework and resources travel with the package. In code:

```swift
import Rhubarb

try Rhubarb.useBundledResources()
```

`useBundledResources()` looks up `res/` in `Bundle.module` (which SPM populates from `swift/Sources/Rhubarb/Resources/`).

If you want to vendor the package into your own repo instead of pointing at this checkout, copy `swift/Package.swift`, `swift/Sources/`, and `swift/Rhubarb.xcframework` into your project. Don't forget `Sources/Rhubarb/Resources/res/`.

### Path B — Drag-in to an Xcode project (no SwiftPM)

Best for an existing app that doesn't use Swift packages.

1. Drag `swift/Rhubarb.xcframework` into your Xcode project navigator.
2. In your app target's **General** tab → **Frameworks, Libraries, and Embedded Content**, set Rhubarb's embed mode to **Do Not Embed** (it's a static library).
3. Drag `swift/Sources/Rhubarb/Resources/res` into your project as a folder reference (blue folder, **Create folder references** — not groups). Add it to your target's **Copy Bundle Resources** build phase.
4. Copy `swift/Sources/Rhubarb/Rhubarb.swift` into your project (the typed Swift overlay). Or write your own — the C ABI is documented in `Headers/Rhubarb.h` inside the xcframework and is callable directly from Swift via `import RhubarbC`.

At runtime, point Rhubarb at the resources:

```swift
import Rhubarb

if let resURL = Bundle.main.url(forResource: "res", withExtension: nil) {
    Rhubarb.setResourceDirectory(resURL.deletingLastPathComponent())
}
```

`setResourceDirectory` expects the *parent* of `res/`, i.e. a directory whose `res/sphinx/cmudict-en-us.dict` exists.

---

## 3. Use the API

The same code works for both integration paths.

```swift
import Rhubarb

try Rhubarb.useBundledResources()  // SwiftPM only
// or:
// Rhubarb.setResourceDirectory(URL(fileURLWithPath: "/path/to/dir/with/res"))

var options = AnimationOptions()
options.recognizer     = .pocketSphinx       // or .phonetic
options.dialog         = "Hello, world."     // optional, improves accuracy
options.extendedShapes = "GHIJKLX"           // default "GHX"
options.framerate      = 24                  // 0 = no frame snapping
options.threadCount    = 0                   // 0 = all cores

let audio = URL(fileURLWithPath: "/path/to/recording.wav")
let cues = try Rhubarb.animate(audioFile: audio, options: options)

for cue in cues {
    print(String(format: "%.3f–%.3f  %@", cue.start, cue.end, String(cue.shape.rawValue)))
}
```

### Types at a glance

| Type                | Notes                                                                  |
| ------------------- | ---------------------------------------------------------------------- |
| `Shape`             | Character-backed enum (`A`–`L`, `X`). `Sendable`, `CaseIterable`.      |
| `MouthCue`          | `start: TimeInterval`, `end: TimeInterval`, `shape: Shape`. `Hashable`.|
| `Recognizer`        | `.pocketSphinx` (English, default), `.phonetic` (any language).        |
| `AnimationOptions`  | All knobs above. `Sendable`.                                           |
| `RhubarbError`      | Throws on bad input, missing file, recognition failure, internal bug.  |
| `Rhubarb.version`   | Returns the Rhubarb version string.                                    |

### Audio formats

`audioFile` may be either a `.wav` or `.ogg` (Ogg Vorbis) file. Anything else fails with `RhubarbError.fileNotFound` or `.recognitionFailed`. There is no built-in transcoding — convert formats up front (e.g. with `AVAssetExportSession` or `ffmpeg`) if needed.

### Threading

`Rhubarb.animate(...)` is **synchronous and CPU-bound**. Call it from a background queue or `Task.detached { ... }` to keep the main thread free:

```swift
let cues = try await Task.detached(priority: .userInitiated) {
    try Rhubarb.animate(audioFile: audio, options: options)
}.value
```

---

## 4. App Sandbox notes

If your app uses the App Sandbox (default for App Store distribution):

- **Reading user-selected audio files** works under `com.apple.security.files.user-selected.read-only` (NSOpenPanel grants the scoped URL automatically).
- **Reading the resource bundle** is fine — it's inside your app bundle.
- **No network access** is required at runtime; PocketSphinx is fully offline.

There are no special entitlements specifically for Rhubarb.

---

## 5. Common pitfalls

**`Rhubarb.useBundledResources()` throws `fileNotFound`.**  
The Swift package was built but `package-xcframework.sh` didn't populate `swift/Sources/Rhubarb/Resources/res/`. Re-run the script.

**Linker errors like `Undefined symbols: _animate...`.**  
The xcframework wasn't actually linked to your target. Check **Frameworks, Libraries, and Embedded Content** — Rhubarb should be listed and embed mode should be **Do Not Embed** (static).

**`error: no such module 'Rhubarb'` at compile time.**  
For SwiftPM: the package wasn't added to your target's dependencies. For drag-in: you didn't include `Rhubarb.swift` in your target. Note the C-side module is named `RhubarbC`; the Swift overlay re-exports it as `Rhubarb`.

**`Building for x86_64 but library is arm64-only` (or vice versa).**  
The framework already contains both arches. Verify with `lipo -archs swift/Rhubarb.xcframework/macos-arm64_x86_64/librhubarb.a` — should print `x86_64 arm64`. If not, rerun `package-xcframework.sh`.

**Crash inside PocketSphinx during the first `animate()` call** with a message like *"could not find resource file ... cmudict-en-us.dict"*.  
You forgot to call `setResourceDirectory` (or `useBundledResources`) before `animate`. The C side has no fallback.

**`RhubarbError.unknownShape('?')`.**  
Indicates a shape character the Swift overlay didn't recognize. This means the C-side `Shape` enum gained a new value not yet mapped in `Rhubarb.swift`. Update the Swift `Shape` enum.

---

## 6. Updating the framework

When pulling new Rhubarb commits, rerun:

```bash
./package-xcframework.sh
```

For SwiftPM consumers: Xcode will pick up the new xcframework on its next build (you may need to clean derived data). For drag-in consumers: replace the dropped-in `Rhubarb.xcframework` and `res/` folders.
