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
    public var recognizer: Recognizer = .pocketSphinx
    public var dialog: String? = nil
    public var extendedShapes: String = "GHX"
    public var framerate: Int = 0
    public var threadCount: Int = 0

    public init(
        recognizer: Recognizer = .pocketSphinx,
        dialog: String? = nil,
        extendedShapes: String = "GHX",
        framerate: Int = 0,
        threadCount: Int = 0
    ) {
        self.recognizer = recognizer
        self.dialog = dialog
        self.extendedShapes = extendedShapes
        self.framerate = framerate
        self.threadCount = threadCount
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

        return try options.dialog.withCStringOrNull { dialogPtr in
            try options.extendedShapes.withCString { shapesPtr in
                cOptions.dialog = dialogPtr
                cOptions.extended_shapes = shapesPtr

                return try audioFile.path.withCString { pathPtr in
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
