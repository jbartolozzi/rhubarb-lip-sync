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
            resources: [.copy("Resources")]
        ),
        .testTarget(
            name: "RhubarbTests",
            dependencies: ["Rhubarb"],
            path: "Tests/RhubarbTests"
        ),
    ]
)
