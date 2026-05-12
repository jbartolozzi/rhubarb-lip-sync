import Foundation
import RhubarbC

public enum Shape: Character, Sendable, CaseIterable {
    case a = "A", b = "B", c = "C", d = "D", e = "E", f = "F"
    case g = "G", h = "H", i = "I", j = "J", k = "K", l = "L"
    case x = "X"
}

public struct MouthCue: Sendable, Hashable {
    public let start: TimeInterval
    public let end: TimeInterval
    public let shape: Shape

    public init(start: TimeInterval, end: TimeInterval, shape: Shape) {
        self.start = start
        self.end = end
        self.shape = shape
    }
}

public enum Recognizer: Sendable {
    case pocketSphinx
    case phonetic

    fileprivate var cValue: RhubarbRecognizer {
        switch self {
        case .pocketSphinx: return RHUBARB_RECOGNIZER_POCKET_SPHINX
        case .phonetic:     return RHUBARB_RECOGNIZER_PHONETIC
        }
    }
}

public struct AnimationOptions: Sendable {
    /// One frame of an optional ARKit blendshape track aligned to the
    /// audio. `values` holds the per-frame weights; the count and meaning
    /// of each slot is up to the caller (typically 52 ARKit blendshapes
    /// in their canonical order). All frames in `blendshapes` must agree
    /// on `values.count`.
    ///
    /// Nested inside AnimationOptions so the name doesn't collide with
    /// consumers (e.g. apps that have their own `BlendshapeFrame` type).
    public struct BlendshapeFrame: Sendable {
        public var timestampSeconds: Double
        public var values: [Float]

        public init(timestampSeconds: Double, values: [Float]) {
            self.timestampSeconds = timestampSeconds
            self.values = values
        }
    }

    public var recognizer: Recognizer = .pocketSphinx
    public var dialog: String? = nil
    public var extendedShapes: String = "GHX"
    public var framerate: Int = 0
    public var threadCount: Int = 0

    /// Optional blendshape track aligned to the audio file passed to
    /// `animate(...)`. When non-nil and non-empty, rhubarb's enhanced
    /// recognizers may use it to refine cue selection. Nil means
    /// audio-only behavior, unchanged from upstream.
    public var blendshapes: [BlendshapeFrame]? = nil

    /// Blendshape-driven VAD gating. When true (and `blendshapes` is
    /// non-empty) the recognizer intersects its audio VAD output with
    /// a jawOpen-derived mouth-motion mask, suppressing utterances
    /// where the face wasn't moving. Defaults to false so existing
    /// callers see no behavior change.
    public var useBlendshapeVad: Bool = false

    /// Per-frame jawOpen weight above which the mouth is considered to
    /// be moving. Only used when `useBlendshapeVad` is true.
    public var vadJawOpenThreshold: Float = 0.05

    /// Minimum gap between mouth-motion runs to count as silence (ms).
    /// Shorter gaps are merged so brief lip closures during /p/, /b/,
    /// /m/ don't fragment an utterance. Only used when
    /// `useBlendshapeVad` is true.
    public var vadMinSilenceMs: Int = 200

    public init(
        recognizer: Recognizer = .pocketSphinx,
        dialog: String? = nil,
        extendedShapes: String = "GHX",
        framerate: Int = 0,
        threadCount: Int = 0,
        blendshapes: [BlendshapeFrame]? = nil,
        useBlendshapeVad: Bool = false,
        vadJawOpenThreshold: Float = 0.05,
        vadMinSilenceMs: Int = 200
    ) {
        self.recognizer = recognizer
        self.dialog = dialog
        self.extendedShapes = extendedShapes
        self.framerate = framerate
        self.threadCount = threadCount
        self.blendshapes = blendshapes
        self.useBlendshapeVad = useBlendshapeVad
        self.vadJawOpenThreshold = vadJawOpenThreshold
        self.vadMinSilenceMs = vadMinSilenceMs
    }
}

public enum RhubarbError: Error, CustomStringConvertible {
    case invalidArgument(String)
    case fileNotFound(String)
    case recognitionFailed(String)
    case `internal`(String)
    case unknownShape(Character)

    public var description: String {
        switch self {
        case .invalidArgument(let m), .fileNotFound(let m),
             .recognitionFailed(let m), .internal(let m): return m
        case .unknownShape(let c): return "Unknown shape character: \(c)"
        }
    }
}

public enum Rhubarb {
    /// Point Rhubarb at the directory containing `res/sphinx/cmudict-en-us.dict`.
    /// Call this once before the first `animate(...)` invocation, or every time
    /// the resource location may have changed.
    public static func setResourceDirectory(_ url: URL) {
        url.path.withCString { rhubarb_set_resource_directory($0) }
    }

    /// Use the resource bundle that Swift Package Manager copies into
    /// `Bundle.module`. Requires the consuming target to have included
    /// `Resources/res/sphinx/...` (the layout produced by
    /// `package-xcframework.sh`).
    public static func useBundledResources() throws {
        guard let resURL = Bundle.module.url(forResource: "res", withExtension: nil) else {
            throw RhubarbError.fileNotFound(
                "Bundled `res` directory not found in Bundle.module")
        }
        setResourceDirectory(resURL.deletingLastPathComponent())
    }

    public static var version: String {
        String(cString: rhubarb_version())
    }

    public static func animate(
        audioFile: URL,
        options: AnimationOptions = AnimationOptions()
    ) throws -> [MouthCue] {
        var cOptions = RhubarbOptions()
        rhubarb_default_options(&cOptions)
        cOptions.recognizer   = options.recognizer.cValue
        cOptions.framerate    = Int32(options.framerate)
        cOptions.thread_count = Int32(options.threadCount)
        cOptions.vad_use_blendshape_mask = options.useBlendshapeVad ? 1 : 0
        cOptions.vad_jaw_open_threshold = options.vadJawOpenThreshold
        cOptions.vad_min_silence_ms = Int32(options.vadMinSilenceMs)

        // Flatten blendshape frames into two parallel buffers. The C ABI
        // wants row-major float values plus a parallel timestamps array,
        // both with a lifetime that extends across rhubarb_animate. The
        // nested withUnsafeBufferPointer calls below keep them alive.
        let flat = options.blendshapes.flatMap(flattenBlendshapes)

        return try options.dialog.withCStringOrNull { dialogPtr in
            try options.extendedShapes.withCString { shapesPtr in
                cOptions.dialog = dialogPtr
                cOptions.extended_shapes = shapesPtr

                return try audioFile.path.withCString { pathPtr in
                    return try withBlendshapeBuffers(flat) { tsPtr, vPtr, frameCount, valuesPerFrame in
                        cOptions.blendshape_timestamps = tsPtr
                        cOptions.blendshape_values = vPtr
                        cOptions.blendshape_frame_count = frameCount
                        cOptions.blendshape_values_per_frame = valuesPerFrame

                        var cuesPtr: UnsafeMutablePointer<RhubarbCue>? = nil
                        var count: Int = 0

                        let status = rhubarb_animate(pathPtr, &cOptions, &cuesPtr, &count)
                        if status != RHUBARB_OK {
                            let message = String(cString: rhubarb_last_error())
                            throw mapError(status, message: message)
                        }

                        defer { rhubarb_free_cues(cuesPtr) }

                        guard count > 0, let cuesBase = cuesPtr else { return [] }
                        let buffer = UnsafeBufferPointer(start: cuesBase, count: count)

                        return try buffer.map { raw -> MouthCue in
                            let scalar = UnicodeScalar(UInt8(bitPattern: raw.shape))
                            let ch = Character(scalar)
                            guard let shape = Shape(rawValue: ch) else {
                                throw RhubarbError.unknownShape(ch)
                            }
                            return MouthCue(
                                start: raw.start_seconds,
                                end:   raw.end_seconds,
                                shape: shape)
                        }
                    }
                }
            }
        }
    }
}

// MARK: - Blendshape marshaling

// Side-by-side arrays produced by `flattenBlendshapes`. `values` is
// row-major (frame-major): values[i * valuesPerFrame + j] is the j-th
// weight of frame i.
private struct FlatBlendshapes {
    let timestamps: [Double]
    let values: [Float]
    let valuesPerFrame: Int
}

private func flattenBlendshapes(_ frames: [AnimationOptions.BlendshapeFrame]) -> FlatBlendshapes? {
    guard let first = frames.first, !first.values.isEmpty else { return nil }
    let valuesPerFrame = first.values.count
    var timestamps: [Double] = []
    var values: [Float] = []
    timestamps.reserveCapacity(frames.count)
    values.reserveCapacity(frames.count * valuesPerFrame)
    for f in frames {
        // All frames must agree on valuesPerFrame — otherwise the C side
        // would read past the buffer or misalign rows. Inconsistent
        // frames drop the entire track rather than corrupt it silently.
        guard f.values.count == valuesPerFrame else { return nil }
        timestamps.append(f.timestampSeconds)
        values.append(contentsOf: f.values)
    }
    return FlatBlendshapes(
        timestamps: timestamps,
        values: values,
        valuesPerFrame: valuesPerFrame)
}

// Hands the flattened buffers' base addresses + sizes to `body`, keeping
// the underlying storage alive for the call duration. When `flat` is
// nil, calls `body` with all-zero pointers/sizes so the caller can wire
// "no blendshape data" through without branching.
//
// Doesn't take cOptions inout — Swift's exclusive-access rule forbids
// nesting another &cOptions (for rhubarb_animate) inside an outer
// inout-borrowed scope of the same variable. The caller assigns the
// fields itself inside the body.
private func withBlendshapeBuffers<R>(
    _ flat: FlatBlendshapes?,
    _ body: (UnsafePointer<Double>?, UnsafePointer<Float>?, Int, Int) throws -> R
) rethrows -> R {
    guard let flat else { return try body(nil, nil, 0, 0) }
    return try flat.timestamps.withUnsafeBufferPointer { tsBuf in
        try flat.values.withUnsafeBufferPointer { vBuf in
            try body(
                tsBuf.baseAddress,
                vBuf.baseAddress,
                flat.timestamps.count,
                flat.valuesPerFrame)
        }
    }
}

private func mapError(_ status: RhubarbStatus, message: String) -> RhubarbError {
    switch status {
    case RHUBARB_ERROR_INVALID_ARGUMENT:    return .invalidArgument(message)
    case RHUBARB_ERROR_FILE_NOT_FOUND:      return .fileNotFound(message)
    case RHUBARB_ERROR_RECOGNITION_FAILED:  return .recognitionFailed(message)
    default:                                return .internal(message)
    }
}

private extension Optional where Wrapped == String {
    func withCStringOrNull<R>(_ body: (UnsafePointer<CChar>?) throws -> R) rethrows -> R {
        switch self {
        case .none: return try body(nil)
        case .some(let s): return try s.withCString { try body($0) }
        }
    }
}
