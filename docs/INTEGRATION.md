# StringGuard Integration Guide

**English** · [Türkçe](INTEGRATION.tr.md)

This guide takes an existing Kotlin project through a complete StringGuard
integration: toolchain requirements, installation, per-platform
module setup, the first build, obfuscation scope control, CI and troubleshooting.
It is written for the developer performing the integration. The
[README](../README.md) carries the condensed version.

**Contents**
1. [Requirements](#1-requirements)
2. [Installation](#2-installation)
   - [Declaring the repositories](#21-declaring-the-repositories)
3. [Applying StringGuard](#3-applying-stringguard)
   - [Plain Android](#31-plain-android-classic-non-kmp)
   - [JVM / Desktop](#32-jvm--desktop)
   - [Kotlin Multiplatform / Compose Multiplatform](#33-kotlin-multiplatform--compose-multiplatform)
   - [iOS: linking the framework](#34-ios-linking-the-framework)
   - [Flutter](#35-flutter)
   - [Written directly in Swift](#36-applications-written-directly-in-swift)
4. [First build](#4-first-build)
5. [Configuration](#5-configuration)
6. [Debug, release and CI](#6-debug-release-and-ci)
7. [Binary verification](#7-binary-verification)
8. [Licensing](#8-licensing)
   - [Supplying the key](#81-supplying-the-key)
   - [Without a valid key](#82-what-happens-without-a-valid-key)
   - [Renewal and replacement](#83-renewal-and-replacement)
9. [App Store submission](#9-app-store-submission)
10. [Migrating an existing codebase](#10-migrating-an-existing-codebase)
11. [Troubleshooting](#11-troubleshooting)
12. [FAQ](#12-faq)

## 1. Requirements

| Tool | Version |
|---|---|
| JDK | 17–24 (21 recommended). **Not 25/26**: Gradle 8.14 rejects Java > 24. |
| Gradle | 8.x (tested on 8.14) |
| Kotlin | **2.2.0**, which must match the plugin's compiler build exactly |
| AGP | 8.x (Android) |

The compiler plugin is tied to the Kotlin version. If your project is not on
Kotlin 2.2.0, align it before you start, or wait for a StringGuard build matching
your Kotlin version.

Obfuscate-all is the default. You do not opt literals in, you opt them out.
Literals that have to remain compile-time constants, such as `const val`
initializers and annotation arguments, are never touched, so enabling the plugin
across a whole project does not break code that depends on them.

## 2. Installation

### 2.1 Declaring the repositories

1. In your project's `settings.gradle.kts`, make the repository reachable from
   **both** blocks:

   ```kotlin
   pluginManagement {
       repositories {
           gradlePluginPortal()
           google()
           mavenCentral()
           maven { url = uri("https://duranun.github.io/stringguard/maven") }
       }
   }
   dependencyResolutionManagement {
       repositories {
           google()
           mavenCentral()
           maven { url = uri("https://duranun.github.io/stringguard/maven") }
       }
   }
   ```

   No credentials are needed.

2. Sync the project. Gradle resolves the plugin itself through
   `pluginManagement`, and the `annotations` and `runtime` libraries that the
   plugin adds for you through `dependencyResolutionManagement`. Both blocks
   need the entry; omitting either one produces one of the resolution failures
   listed in [section 11](#11-troubleshooting).

## 3. Applying StringGuard

Find your project below. Sections 3.1 to 3.4 cover Kotlin projects and use the
Gradle plugin; sections 3.5 and 3.6 cover Flutter and plain Swift, which are
configured differently.

| Your project | Section | How literals are protected |
|---|---|---|
| Android app or library, not multiplatform | [3.1](#31-plain-android-classic-non-kmp) | Gradle plugin, every literal in the module |
| JVM or Desktop | [3.2](#32-jvm--desktop) | Gradle plugin, every literal in the module |
| Kotlin Multiplatform or Compose Multiplatform (Android, iOS, JVM, JS, Wasm) | [3.3](#33-kotlin-multiplatform--compose-multiplatform) | Gradle plugin, every literal, every target |
| An iOS app consuming a Kotlin framework | [3.3](#33-kotlin-multiplatform--compose-multiplatform) + [3.4](#34-ios-linking-the-framework) | As 3.3; 3.4 is for the optional guard and attestation |
| Flutter or Dart | [3.5](#35-flutter) | Code generator, from a secrets file |
| Written directly in Swift, no Kotlin | [3.6](#36-applications-written-directly-in-swift) | Xcode build step that rewrites your sources |

For the Kotlin sections: apply StringGuard next to whichever Kotlin plugin the
module already uses. Order does not matter, because StringGuard attaches to the
Kotlin plugin whenever that plugin is applied. Do not add `annotations` or
`runtime` by hand; the plugin adds them to the correct configuration or source
set.

### 3.1 Plain Android (classic, non-KMP)

```kotlin
// app/build.gradle.kts
plugins {
    id("com.android.application")      // or com.android.library
    kotlin("android")
    id("com.stringguard") version "1.0.0"
}

android {
    namespace = "com.example.app"
    compileSdk = 35
    defaultConfig { minSdk = 24 }
    buildTypes {
        // Recommended. StringGuard works either way, but see the note below.
        release { isMinifyEnabled = true }
    }
}

stringGuard {
    enabled = true
    minLength = 4
    licenseKey = providers.environmentVariable("STRINGGUARD_LICENSE").getOrElse("")
}
```

Dependencies are added to the `implementation` configuration.

**Leave minification on for release builds.** StringGuard does not need R8 and
works without it, but R8 renames the runtime protection's internal symbols —
the detector entry points and the decrypt entry point — and StringGuard ships
the rules that let it. A release built without minification leaves those names
readable in the APK, which tells someone decompiling it exactly where the
checks are. You already own that hardening; this is only about not switching it
off.

### 3.2 JVM / Desktop

```kotlin
// build.gradle.kts
plugins {
    kotlin("jvm") version "2.2.0"
    id("com.stringguard") version "1.0.0"
}

stringGuard {
    enabled = true
    minLength = 4
    licenseKey = providers.environmentVariable("STRINGGUARD_LICENSE").getOrElse("")
}
```

Dependencies are added to the `implementation` configuration.

### 3.3 Kotlin Multiplatform / Compose Multiplatform

```kotlin
// build.gradle.kts
plugins {
    kotlin("multiplatform") version "2.2.0"
    id("com.stringguard") version "1.0.0"
}

kotlin {
    androidTarget()
    jvm()
    iosArm64(); iosSimulatorArm64()
    js(IR) { nodejs() }
    wasmJs { nodejs() }
    // ...
}

stringGuard {
    enabled = true
    minLength = 4
    licenseKey = providers.environmentVariable("STRINGGUARD_LICENSE").getOrElse("")
}
```

The `annotations` and `runtime` dependencies are added to `commonMain`, so every
target is reached by the same configuration, Apple and Wasm targets included. No
per-target wiring is required. Every target is obfuscated the same way: there is
one licence and one feature set, so Apple, JS and Wasm targets are not treated
differently from JVM and Android.

**JS and Wasm** need nothing beyond the above. The runtime is pure Kotlin, so the
same protected value decrypts identically on `js(IR)` and `wasmJs` as it does on
the JVM. Browser and Node targets behave the same.

**Compose Multiplatform** is covered by this section as it stands: it is a
Multiplatform project, and the plugin does not treat Compose specially. Literals
inside composable functions are obfuscated like any others.

### 3.4 iOS: linking the framework

String obfuscation on an iOS target needs nothing beyond section 3.3. This
section is about the optional runtime guard and attestation client, which reach
Swift through a Kotlin framework.

**Read this before you add it to an existing multiplatform app.** Kotlin/Native
embeds its own runtime in every framework it produces, so an iOS app cannot link
two of them. `StringGuard.framework` carries both the runtime guard and the
attestation client in one framework for exactly that reason.

`StringGuard.xcframework` is distributed as part of your licence, which names
where to obtain it; it carries both device and simulator slices, with debug
symbols.

If your app does **not** already have its own shared Kotlin framework, link it
the normal way — CocoaPods (`vendored_frameworks`), Swift Package Manager
(`.binaryTarget`) or a manual embed all work.

If your app **already ships its own shared Kotlin framework**, do not link
`StringGuard.framework` alongside it. Add the libraries to your own framework
instead:

```kotlin
// your existing shared module
kotlin {
    sourceSets.commonMain.dependencies {
        api("com.stringguard:guard:<version>")        // api, NOT implementation
        api("com.stringguard:attest-core:<version>")
    }
    listOf(iosArm64(), iosSimulatorArm64(), iosX64()).forEach {
        it.binaries.framework {
            baseName = "Shared"
            export("com.stringguard:guard:<version>")
            export("com.stringguard:attest-core:<version>")
        }
    }
}
```

It must be `api`, not `implementation`: only an `api` dependency can be
exported, and without the export the symbols never reach the generated headers.

For an application written **directly in Swift**, with no Kotlin at all, see
[section 3.6](#36-applications-written-directly-in-swift) instead. That path is
configured differently and has its own defaults and exclusion syntax.

### 3.5 Flutter

Flutter uses a **different model** from the Kotlin and Swift paths, and the
difference matters: it does not obfuscate literals already written in your
source. Instead the plaintext lives in a secrets file outside your source tree,
and a code generator reads it at build time and emits obfuscated payloads. Your
source never contains the secret, and the secrets file is never committed.

Three packages, each with a distinct job:

| Package | Role | Where it goes |
|---|---|---|
| `stringguard` | The annotations and the runtime that reveals values | `dependencies` |
| `stringguard_generator` | The `build_runner` generator that reads the secrets file and emits the payloads | `dev_dependencies` |
| `stringguard_flutter` | A plugin exposing the runtime guard and attestation to Dart. **No string obfuscation.** Optional. | `dependencies`, only if you want those |

In your `pubspec.yaml` (your licence names where to obtain the packages; they are
not on pub.dev):

```yaml
dependencies:
  stringguard: <as your licence names>

dev_dependencies:
  stringguard_generator: <as your licence names>
  build_runner: ^2.4.0
```

Declare the secrets. Keep `secrets.env` out of version control:

```dart
import 'package:stringguard/annotations.dart';
import 'package:stringguard/stringguard.dart';

part 'config.g.dart';

@StringGuardEnv('secrets.env')
class Config {
  @Secret()                       // looked up by field name
  static final String apiKey = _$Config.apiKey;

  @Secret('API_BASE_URL')         // or by an explicit key
  static final String baseUrl = _$Config.baseUrl;
}
```

Then generate:

```bash
dart run build_runner build      # or `watch` while developing
```

The accessor is always `_$<ClassName>.<fieldName>`, declared in the generated
part file. Run the generator before the first compile, or the part file will not
exist yet. No builder configuration is needed: the generator applies
automatically to any package that depends on it.

If you add `stringguard_flutter` for the runtime guard or attestation, note that
it ships its own iOS privacy manifest; see
[section 9](#9-app-store-submission).

### 3.6 Applications written directly in Swift

For an app with no Kotlin at all, a separate integration rewrites your Swift
sources during the Xcode build, so you keep writing ordinary string literals.

Three steps, covered in full in [swift/README.md](../swift/README.md):

1. Add the Swift package and link the `StringGuardRuntime` product.
2. Add a **Run Script** build phase **before** Compile Sources, which rewrites
   your sources into the build directory and writes a virtual-filesystem
   overlay.
3. In Build Settings, point the compiler at that overlay through **Other Swift
   Flags**: `-vfsoverlay $(DERIVED_FILE_DIR)/stringguard/overlay.yaml`.

Your own source files are never modified; the compiler reads rewritten copies.

**This path is configured differently from the Kotlin one, so do not assume one
policy covers a mixed codebase.** The differences that change what you must do:

| | Kotlin | Swift |
|---|---|---|
| Minimum literal length | `minLength`, default 1 | a `--min-length` flag, default **4** |
| Excluding something | `@DontObfuscate` / `@KeepPlain`, `keepRules` | a `// stringguard:keep` comment, or an excluded path |
| Interpolated literals | obfuscated | **not rewritten** |
| Multi-line and raw literals | obfuscated | **not rewritten** |

Interpolated literals being out of scope is the one to plan around: a secret
assembled by interpolation stays readable. Keep secrets as whole literals on
this path. Platform minimums and the full list of what is never rewritten are in
[swift/README.md](../swift/README.md).

## 4. First build

Build the project the way you normally would:

```bash
# Android
./gradlew assembleRelease

# JVM / KMP
./gradlew build
```

Your source code does not change. You keep writing ordinary string literals:

```kotlin
object Config {
    val apiBaseUrl = "https://api.example.com/v2"   // obfuscated in the release binary
    val analyticsKey = "ak_prod_1a2b3c4d5e"         // obfuscated
}
```

At runtime the values are identical to what you wrote; in the binary they are
absent. Confirm this with the checks in [section 7](#7-binary-verification).

## 5. Configuration

The `stringGuard { }` block accepts the following options. Defaults apply when
the option is not set.

| Option | Default | Effect |
|---|---|---|
| `enabled` | `true` | Master switch. When false the compiler plugin is a no-op. |
| `obfuscateAll` | `true` | Obfuscate every eligible literal project-wide. When false, only `@Obfuscate` scopes are obfuscated. |
| `minLength` | `1` | Literals shorter than this many characters stay plain. |
| `enabledForDebug` | `false` | Obfuscate debug build types as well. Test compilations are skipped either way. |
| `algorithm` | `StringGuardAlgorithm.AES_GCM` | AEAD primitive. The alternative is `StringGuardAlgorithm.CHACHA20_POLY1305`. |
| `licenseKey` | `""` | **Required.** Offline license token, or a path to a license file relative to the module directory. Missing, malformed or expired fails the build. See [section 8](#8-licensing). |
| `keepRules { }` | no rules | Package and regular-expression exclusions, described below. |

### Scope exclusion with `@DontObfuscate`

Annotate a file, class, object, function, property or constructor to keep every
literal inside it readable:

```kotlin
import com.stringguard.annotations.DontObfuscate

@DontObfuscate                        // file, class, object, function, property or constructor
object ReflectionKeys {
    const val COLUMN = "user_id"      // (const val would be skipped anyway)
    val lookupName = "com.example.Thing"   // read by Class.forName, so it must stay readable
}
```

`@KeepPlain` has the same effect and is honored by the same code path. It differs
only in its annotation targets: `@DontObfuscate` can be applied to a whole file,
`@KeepPlain` cannot.

### Pattern-based exclusions with `keepRules`

```kotlin
stringGuard {
    minLength = 4
    keepRules {
        keepPackage("com.example.resources.*")   // package + all subpackages stay plain
        keepMatching("^https://cdn\\.")           // literals matching regex stay plain
        keepMatching("^android\\.intent\\.")      // e.g. system intent action strings
    }
}
```

`keepPackage("a.b.*")` keeps package `a.b` and every subpackage below it.
`keepMatching(regex)` keeps any literal whose content matches, evaluated with
`containsMatchIn`.

### Literals that are never obfuscated

The plugin always leaves the following as compile-time constants, because
obfuscating them would not compile. No rules are needed for them.

- `const val` initializers
- Annotation arguments (`@SerialName("…")`, `@GET("…")`, `@Deprecated("…")`, …)
- Annotation class default values

This is why Retrofit, kotlinx.serialization and Room keep working with
obfuscate-all on.

### Opt-in mode

To obfuscate only explicitly marked scopes, turn off project-wide obfuscation:

```kotlin
stringGuard { obfuscateAll = false }
```

Then annotate the scopes you want covered:

```kotlin
import com.stringguard.annotations.Obfuscate

@Obfuscate
class PaymentClient {
    private val merchantSecret = "msk_live_..."   // obfuscated
}
// everything outside @Obfuscate scopes stays plain
```

## 6. Debug, release and CI

Three rules govern which compilations are obfuscated:

- Test compilations are always skipped and are never obfuscated. These are the
  compilations Kotlin and AGP name `test`, `testFixtures`, or anything ending in
  `Test` -- `jvmTest`, `debugUnitTest`, `freeDebugAndroidTest`.
- Debug build types are left plain unless `enabledForDebug = true`. This covers
  `debug` and every flavored debug variant (`prodDebug`, `freeDebug`).
- Everything else is obfuscated, including custom build types such as `staging`
  or `qa`. `enabledForDebug` cannot leave those plain; use a keep rule.

Classification is by exact name and suffix, never by substring, so a product
flavor whose name merely contains "test" or "debug" -- `latest`, `contest`,
`fastest` -- is not mistaken for a test or debug compilation.

```kotlin
stringGuard {
    enabledForDebug = false   // keep dev builds fast and logs greppable
}
```

Debug builds stay plain by default because obfuscation adds a small per-literal
cost and makes `strings` output and logcat harder to read during development.
The shipping configuration is the one that needs protection.

CI needs the licence and a pinned JDK; nothing else. A typical release pipeline
runs:

```bash
export STRINGGUARD_LICENSE="<injected from your CI secret store>"
./gradlew clean assembleRelease      # or :app:bundleRelease
```

Inject the token as a CI secret and never commit it. The build fails when it is
missing, so a pipeline that forgets it cannot quietly ship unprotected strings.

If CI uses a container image with JDK 25 or 26, pin a JDK in the 17–24 range;
see [section 11](#11-troubleshooting). Add the plaintext-leak check from the
next section as a build gate.

## 7. Binary verification

Protected literals do not appear in the compiled output. Every StringGuard
release is blocked unless that is demonstrated on each supported backend, so
this is a property of the release rather than something you need to establish.

If you want to see it in your own build, the JVM output is the quickest place
to look:

```bash
./gradlew build
javap -c -p -classpath build/libs/<your>.jar com.example.Config
# the secret literals are gone
```

A raw text search over a compiled binary is not a dependable check on every
platform and should be treated as inconclusive rather than as a result.

## 8. Licensing

A licence is **required**. StringGuard has one feature set and one kind of key:
a valid licence enables everything, on every target, and there is no free, trial
or intermediate tier.

Verification is offline. The token is checked against a public key embedded in
the plugin, at configuration time, with no network call, so it works in CI and
in air-gapped builds.

### 8.1 Supplying the key

```kotlin
stringGuard {
    licenseKey = "<token>"      // or a path to a license file (relative to the module)
}
```

The token is confidential, so keep it out of version control. Reading it from
the environment is the usual arrangement, and keeps it out of the Gradle
configuration cache entry as well:

```kotlin
stringGuard {
    licenseKey = providers.environmentVariable("STRINGGUARD_LICENSE")
        .orElse(providers.gradleProperty("stringguard.license"))
        .getOrElse("")
}
```

In CI, inject it as a secret. A file works too, which suits a key mounted by a
secret manager:

```kotlin
stringGuard { licenseKey = "config/stringguard.license" }
```

### 8.2 What happens without a valid key

**The build fails.** Configuration stops with an explanatory error rather than
producing a build that protects less than you asked for:

| Condition | Result |
|---|---|
| `licenseKey` unset, blank, or an empty file | `StringGuard requires a license key. ...` |
| Not a StringGuard token, truncated, or signed by another key | `StringGuard license is not valid: <reason>. ...` |
| Issued in the retired v1 format | `... license format v1 is no longer supported; reissue the key ...` |
| Past its expiry day | `StringGuard license '<id>' expired on <date>. ...` |

There is no flag to make this non-fatal. The alternative -- continue and
obfuscate less -- ships a binary with readable secrets from a green build, which
is the one outcome a string obfuscator must never produce.

A module that genuinely should not be obfuscated opts out and then needs no key:

```kotlin
stringGuard { enabled = false }
```

A successful build logs the licence once, at `lifecycle` level:

```
StringGuard: license verified (holder=Acme Inc, expires=2027-04-01)
```

Note that on a Gradle **configuration-cache hit** the project is not configured
again, so the licence is not re-verified and that line is not printed. A licence
that expires between two cached builds is caught on the next cold
configuration.

### 8.3 Renewal and replacement

A licence is issued for a stated period, and renewing it means replacing the
token your build reads. If you supply it from the environment or from a file, as
in [8.1](#81-supplying-the-key), renewal is a secret-store change with no code
change and no release.

Plan the renewal with margin. Because verification is fatal, an expired key
stops builds — including a hotfix build during an incident. Give the licence a
named owner and an alert weeks ahead of its expiry date, and keep the renewal
path somewhere your on-call engineer can find it.

A licence is valid **through its expiry date inclusive**, and the date is
evaluated in **UTC** by both the issuer and the build. So a key dated
2027-04-01 still builds on 2027-04-01 wherever your agents run, and stops on
2027-04-02. Leave margin anyway; do not schedule a renewal to land on the
boundary.

Already-released applications are unaffected: runtime components you have
already shipped stay licensed for your end users. Only your ability to produce
new builds stops.

If you need a key for a new team, a different build system, or a replacement
after a leak, request it through your usual commercial contact. Treat a leaked
token as you would any other credential: have it replaced, and rotate the secret
in every pipeline that held it.

## 9. App Store submission

Two App Store obligations follow from embedding StringGuard in an iOS
application. Both are the submitting developer's to answer, and both are
answered with facts rather than defaults.

### Encryption export compliance

StringGuard decrypts protected literals with AES-GCM. On Apple targets that
decryption is performed by StringGuard's own implementation, not by CryptoKit or
another operating-system framework.

Apple's guidance treats encryption built into the operating system, such as
HTTPS through `URLSession`, as typically exempt from the export documentation
requirement, and does not extend that treatment to an implementation shipped
inside the application. So the answer to App Store Connect's encryption question,
and the value of `ITSAppUsesNonExemptEncryption` in `Info.plist`, may differ from
what it was before StringGuard was added. A non-exempt answer can carry a
year-end self-classification report.

This documentation does not tell you which answer is correct for your
application; that depends on your jurisdiction and your other dependencies. It
tells you the fact you need in order to answer: the encryption is in the
application, not in the operating system. See
[Complying with Encryption Export Regulations](https://developer.apple.com/documentation/security/complying-with-encryption-export-regulations).

### Privacy manifest

The Gradle plugin, the compiler plugin and the obfuscation runtime use no
required-reason API, so string obfuscation on its own adds nothing to your
privacy manifest.

**The runtime guard is different.** Its jailbreak check reads file metadata for
paths outside the app container, which falls under Apple's **file-timestamp**
required-reason API category. So if you enable the runtime guard — by linking
`StringGuard.framework`, or by exporting the guard library from your own
framework as in [section 3.4](#34-ios-linking-the-framework) — your
application's `PrivacyInfo.xcprivacy` needs an `NSPrivacyAccessedAPITypes` entry
for that category.

Ask us for the current reason-code guidance before you submit. Apple revises
the required-reason list, and the right declaration depends on your app as well
as on StringGuard, so we keep that guidance current rather than freezing a code
into this document. An undeclared required-reason API usually draws an
`ITMS-91053` notice on upload rather than a rejection, so this is something to
settle before submission, not a blocker.

The Flutter plugin has its own case: it reads and writes one value in
`UserDefaults`, the App
Attest key identifier. `UserDefaults` is a required-reason API, and an
undeclared use produces an `ITMS-91053` notice on upload. The plugin ships its
own `PrivacyInfo.xcprivacy` declaring category
`NSPrivacyAccessedAPICategoryUserDefaults` with reason `CA92.1`, so no
declaration is needed in your application's manifest for it. See
[Describing use of required reason API](https://developer.apple.com/documentation/bundleresources/privacy_manifest_files/describing-use-of-required-reason-api).

## 10. Migrating an existing codebase

1. **Apply the plugin** as described in [section 3](#3-applying-stringguard) and
   build once with `enabled = true`. Obfuscate-all is the default, so everything
   eligible is obfuscated on that first build with no code changes.

2. **Build a release and run your test suite and smoke tests.** The guardrails
   cover `const val` and annotation arguments. If your application reads a
   runtime string literal by identity somewhere unusual, such as reflection over
   a literal or a hand-rolled resource lookup keyed by a literal, exclude it in
   one of two ways:
   - annotate that scope with `@DontObfuscate`, or
   - add a `keepMatching` or `keepPackage` rule.

3. **Raise `minLength`**, for example to `4`, to skip trivial literals and shrink
   the output.

4. **Add the plaintext-leak check** from [section 7](#7-binary-verification) to
   CI, so that a new secret added in plain text is caught automatically.

5. **Switch to the opt-in model if you prefer it.** Set `obfuscateAll = false`
   and annotate the handful of sensitive scopes with `@Obfuscate`.

The following need no action and are safe as they are: Retrofit and Ktor endpoint
annotations, `@Serializable` field names declared via `@SerialName`, logging tags
declared as `const val`, and dependency-injection qualifiers that use annotation
arguments.

## 11. Troubleshooting

**`Incompatible Gradle JVM version … supports Java 1.8 and 24` (Android Studio).**
Android Studio's bundled JBR may be JDK 25, which Gradle 8.14 rejects. Set the
Gradle JDK to a version in the 17–24 range:

- *Settings → Build, Execution, Deployment → Build Tools → Gradle → Gradle JDK*, then pick or add a JDK in the 17–21 range, **or**
- set `org.gradle.java.home=/path/to/jdk21` in `gradle.properties` (and/or `java.home=…` in `.gradle/config.properties`, which is what the IDE's `#GRADLE_LOCAL_JAVA_HOME` option reads),

then run **Sync Project with Gradle Files**. Do not click *"Apply compatible
Gradle JDK configuration"* without checking what it selects, because it may pick
an unsuitably old JDK.

**`Plugin [id: 'com.stringguard'] was not found`.** The repository your licence
names is missing from `pluginManagement.repositories`, or its credentials are
not configured ([section 2.1](#21-declaring-the-repositories)).

**`Could not find com.stringguard:runtime` / `:annotations`.** The repository is
missing from `dependencyResolutionManagement.repositories`. Both repository
blocks need it, not just `pluginManagement`.

**Compiler plugin version or ABI error after a Kotlin bump.** The compiler plugin
is built against a specific Kotlin version, 2.2.0. Keep your project's Kotlin
version matched, or use a StringGuard build compiled against yours.

**A literal you expected to be encrypted is still plaintext.** Check, in this
order, whether it is a `const val`; an annotation argument; shorter than
`minLength`; inside a `@DontObfuscate` or `@KeepPlain` scope; matched by a
`keepRules` pattern; or in a debug build with `enabledForDebug = false`.

**A string is obfuscated but has to stay readable at runtime (reflection).**
Exclude that scope with `@DontObfuscate` or a `keepRules` entry.

## 12. FAQ

**Do I have to annotate anything?** No. Obfuscate-all is the default. The
`@DontObfuscate` and `@KeepPlain` annotations exist for exclusions, and
`@Obfuscate` applies only to the optional opt-in mode.

**Does it work without R8 or ProGuard?** Yes. StringGuard is independent of R8
and runs inside the Kotlin compiler. The two are complementary and can be used
together: R8 renames and shrinks, StringGuard encrypts string contents.

**Does it slow the app down?** Decryption is per-literal, small and synchronous.
The cost is negligible for typical secret and configuration strings. If you
measure a difference, exclude very large or hot-loop literals with `minLength` or
`keepRules`.

**Is this real security?** StringGuard raises the cost of static analysis and
defeats casual `strings` inspection, decompilation and automated scanners. It is
not cryptographic secrecy: decryption happens on the device, so the key is
derivable there. A value that must never leak belongs on a server. The README
describes the full security model.

**Are iOS and Wasm really covered on Multiplatform?** Yes. The runtime is pure
Kotlin and is added to `commonMain`, and the same encrypted payload decrypts
identically on JVM, Kotlin/Native (iOS backend), JS and Wasm. Obfuscation of
Native, JS and Wasm targets needs no separate entitlement: one licence covers
every target.

**Can I change the cipher to ChaCha20?** Yes. Set
`algorithm = StringGuardAlgorithm.CHACHA20_POLY1305` in the `stringGuard { }`
block. The runtime decrypts both AES-GCM and ChaCha20-Poly1305 payloads on every
target, so you can switch freely. AES-GCM is the default.

**What about tamper-binding the key to the app signature?** This is available as
`tamperBinding` (Android only). The README covers the setup, the Play App
Signing note and the limits of the mechanism.
