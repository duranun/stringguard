// swift-tools-version: 5.9
// The swift-tools-version declares the minimum version of Swift required to build this package.

import PackageDescription

let package = Package(
    name: "stringguard_flutter",
    platforms: [
        .iOS("15.0")
    ],
    products: [
        .library(name: "stringguard-flutter", targets: ["stringguard_flutter"])
    ],
    dependencies: [
        .package(name: "FlutterFramework", path: "../FlutterFramework")
    ],
    targets: [
        // Kotlin/Native binary holding guard and attest-core. Produced by
        // ../../build-ios-framework.sh, which must be run before building.
        .binaryTarget(
            name: "StringGuard",
            path: "StringGuard.xcframework"
        ),
        .target(
            name: "stringguard_flutter",
            dependencies: [
                .product(name: "FlutterFramework", package: "FlutterFramework"),
                .target(name: "StringGuard")
            ],
            resources: [
                // Required, not optional: the plugin reads and writes UserDefaults, a
                // required-reason API. Without this an app embedding the plugin is warned
                // on upload (ITMS-91053).
                .process("PrivacyInfo.xcprivacy"),

                // If you have other resources that need to be bundled with your plugin, refer to
                // the following instructions to add them:
                // https://developer.apple.com/documentation/xcode/bundling-resources-with-a-swift-package
            ]
        )
    ]
)
