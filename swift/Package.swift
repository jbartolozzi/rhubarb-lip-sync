// swift-tools-version:5.9
import PackageDescription

let package = Package(
    name: "Rhubarb",
    platforms: [
        .macOS(.v11),
    ],
    products: [
        .library(name: "Rhubarb", targets: ["Rhubarb"]),
    ],
    targets: [
        .binaryTarget(
            name: "RhubarbC",
            path: "Rhubarb.xcframework"
        ),
        .target(
            name: "Rhubarb",
            dependencies: ["RhubarbC"],
            path: "Sources/Rhubarb",
            // Copy the inner `res/` folder so it lands at the bundle root,
            // matching what useBundledResources() expects (`Bundle.module
            // .url(forResource: "res", ...)`). Copying the outer "Resources"
            // folder by name produces a doubled `Resources/Resources/res/`
            // path that the upstream lookup misses.
            resources: [.copy("Resources/res")]
        ),
        .testTarget(
            name: "RhubarbTests",
            dependencies: ["Rhubarb"],
            path: "Tests/RhubarbTests"
        ),
    ]
)
