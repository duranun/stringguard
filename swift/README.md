# StringGuard for native Swift

String obfuscation for applications written directly in Swift, rather than in
Kotlin Multiplatform or Flutter. Every eligible string literal is protected by
default; the developer excludes code rather than marking it.

StringGuard is commercial, proprietary software. See [LICENSE](../LICENSE).
Your licence names where to obtain this package.

## Requirements

| Component | Requirement |
|---|---|
| Swift | 5.9 or later |
| Platforms | iOS 15, macOS 12, tvOS 15, watchOS 8 |
| Runtime dependencies | None. The runtime uses CryptoKit, which is part of the operating system. |

## How it fits your build

Protection happens in a build step that runs before compilation. It reads your
source files, writes protected copies into the build directory, and points the
compiler at those copies. Two properties follow, and both matter day to day:

- **Your sources are never modified.** The step only reads them. Nothing is
  backed up, swapped or restored, so an interrupted build leaves nothing behind
  and your working tree and project file are untouched.
- **Diagnostics stay on your files.** A compiler error on line 42 of your file
  is reported on line 42 of your file, so errors, warnings and breakpoints line
  up as usual. Columns shift on a line that contained a protected literal.

## Setup

### 1. Add the package

```swift
// Your licence names where to obtain the package; a local checkout works too.
.package(url: "<the URL your licence names>", from: "<version>")
```

and link the runtime in your target:

```swift
.product(name: "StringGuardRuntime", package: "StringGuard")
```

### 2. Add a build phase

In your target's Build Phases, add a Run Script **before** Compile Sources:

```bash
"$SG_TOOL" \
  --output "$DERIVED_FILE_DIR/stringguard" \
  --overlay "$DERIVED_FILE_DIR/stringguard/overlay.yaml" \
  $(find "$SRCROOT/YourApp" -name '*.swift')
```

`SG_TOOL` is the path to the rewriter executable from the package.

### 3. Point the compiler at the overlay

In Build Settings, add to **Other Swift Flags**:

```
-vfsoverlay $(DERIVED_FILE_DIR)/stringguard/overlay.yaml
```

## Excluding code

| Mechanism | Effect |
|---|---|
| `// stringguard:keep` | Leaves the literals on the line below, and in the declaration it precedes, readable |
| `--exclude <substring>` | Skips every source path containing the substring, repeatable |
| `--min-length <n>` | Leaves literals shorter than `n` alone; the default is 4 |

## Literals that are never rewritten

These are not policy choices. Each is a position where Swift requires a literal
and a call would not compile, or where rewriting would move a line.

- Enum raw values, including `CodingKeys`. A raw value is metadata the compiler
  stores, not an expression.
- Attribute arguments, such as `@available(message:)` and `@objc("name")`.
- Arguments to `#selector`, `#keyPath` and any other freestanding macro, which
  read the syntax rather than a value.
- `#if` conditions.
- Interpolated strings. The value is already assembled at runtime.
- Multi-line literals. A single-line call in their place would shift every
  following line and break the line mapping.
- Raw literals (`#"..."#`), which are usually patterns or paths kept verbatim.
- Literals shorter than `--min-length`.

## Verifying it

Protected literals do not appear in the built binary, and no StringGuard
release ships unless that is demonstrated on the Swift backend along with every
other one. The check belongs to the build output rather than the source: the
rewrite happens on the way to the compiler, so your checked-in sources are
unchanged by design.

## Payload format

Protected values use AES-256-GCM in the same payload format the Kotlin and Dart
runtimes read, so a value obfuscated on one platform is readable by the runtime
on any of them. The format is covered by cross-platform tests in both
directions.

A fresh key and nonce are used for every value, so two builds of the same source
produce different bytes. That is deliberate: a stable ciphertext would let
someone holding two of your releases confirm which protected values are
unchanged.

AES-GCM and SHA-256 come from CryptoKit here, while the Kotlin and Dart runtimes
implement them in the library, because those runtimes have no system primitive
available on every target they support.

## Scope and limitations

Obfuscation is not encryption. Decryption happens on the end user's device, so
an attacker with full control of that device can recover protected values at
runtime. What it buys is that the plaintext is absent from the artifact, which defeats a `strings` pass, a casual decompile and
automated secret scanners. It does not stop an attacker who attaches a debugger
or instruments the running process. A value that must not be disclosed belongs
on a server you control, not in a client binary.

For App Store submission, see the App Store section of
[docs/INTEGRATION.md](../docs/INTEGRATION.md). The encryption here is CryptoKit,
which is part of the operating system, unlike the Kotlin framework's own
implementation.
