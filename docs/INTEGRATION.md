# StringGuard Integration Guide

**English** · [Türkçe](INTEGRATION.tr.md)

This guide takes an existing Kotlin project through a complete StringGuard
integration: toolchain requirements, installation, per-platform
module setup, the first build, obfuscation scope control, CI, attestation and troubleshooting.
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
10. [Attestation](#10-attestation)
   - [Adding it to your app](#103-adding-it-to-your-app)
   - [Deciding in your backend](#104-deciding-in-your-backend)
   - [Running the service](#107-running-the-service)
11. [Migrating an existing codebase](#11-migrating-an-existing-codebase)
12. [Troubleshooting](#12-troubleshooting)
13. [FAQ](#13-faq)

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
   listed in [section 12](#12-troubleshooting).

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
works without it, but R8 renames the runtime protection's internal symbols, and
StringGuard ships the rules that let it. A release built without minification
leaves those names readable in the APK, which shows someone decompiling it
exactly where the checks are.

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

**JS and Wasm** need nothing beyond the above. A protected value decrypts
identically on `js(IR)` and `wasmJs` as it does on the JVM. Browser and Node targets behave the same.

**Compose Multiplatform** is covered by this section as it stands: it is a
Multiplatform project, and the plugin does not treat Compose specially. Literals
inside composable functions are obfuscated like any others.

### 3.4 iOS: linking the framework

String obfuscation on an iOS target needs nothing beyond section 3.3. This
section is about the optional runtime guard and attestation client, which reach
Swift through a Kotlin framework.

**Read this before you add it to an existing multiplatform app.** An iOS app
cannot link two Kotlin frameworks together, so `StringGuard.framework` carries
the runtime guard and the attestation client in one framework.

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

Flutter has two separate parts. Use either one, or both:

| You want | Packages | How it works |
|---|---|---|
| **String obfuscation** | `stringguard` (`dependencies`) and `stringguard_generator` (`dev_dependencies`) | Your secrets live in a file outside your source. At build time a code generator turns them into obfuscated values your Dart code reads. Set up below. |
| **Runtime guard and attestation** | `stringguard_flutter` (`dependencies`) | A plugin that runs StringGuard's native checks on Android and iOS. Attestation is set up in [10.3.1](#1031-flutter). |

Unlike the Kotlin plugin, the Flutter side does **not** obfuscate string
literals already written in your Dart code. Only values declared from the
secrets file are protected, so move each secret there.

#### Obfuscation

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
see [section 12](#12-troubleshooting). Add the plaintext-leak check from the
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

## 10. Attestation

Optional. Attestation tells your backend, for each app session, whether it comes
from your genuine app on a sound device. The app checks itself against the
attestation service you run in your own infrastructure; your backend reads the
result and decides what to allow.

### 10.1 The results

| Result | When you get it |
|---|---|
| `ALLOW` | Your genuine app, on a device that proved it is genuine. |
| `ALLOW_LIMITED` | Your app, on a device that could not prove it is genuine. Common on older devices and emulators, and usually not an attack. |
| `FLAG` | Your app, but the device showed signs of tampering: a debugger, a hooking framework, a rooted device or a modified system. |
| `DENY` | Not your genuine app, or a check that failed. No session token is issued. |

Keep in mind:

- **A result describes the device at the moment it checked.** The app checks
  again for each session.
- **Attestation does not change how obfuscated strings are protected.** It tells
  your backend about the device; it does not make on-device values safer.

### 10.2 Platform requirements

| | Requirement |
|---|---|
| Android | API 26+ for an `ALLOW` result. Older devices get `ALLOW_LIMITED`. |
| iOS | Apple App Attest, which needs the **App Attest capability** on the app's provisioning profile and a **physical device**. The Simulator and devices without a Secure Enclave report "unsupported" rather than failing. |
| Other targets | Not supported. Calls report "unsupported" rather than erroring. |

Unsupported is a normal branch, not an error: every client call returns a result
you can inspect and none of them throws.

### 10.3 Adding it to your app

Add the client library to your shared module:

```kotlin
// commonMain
implementation("com.stringguard:attest-core:<version>")
```

On iOS, read [section 3.4](#34-ios-linking-the-framework) first if your app
already links its own Kotlin framework — it cannot link two.

```kotlin
// 1. Ask your service for a nonce, then attest. Keep the key so it can sign
//    later requests. On Apple, use the suspending form: App Attest is
//    asynchronous and there is no synchronous path.
val alias = deviceKeyAlias(tenantId)          // see 10.6 if you ship an SDK
val outcome = platformAttestPersistent(nonce, alias)

// 2. Pass the runtime guard's findings along with it.
val r = Guard.report()
val signals = DeviceSignals(r.debuggerAttached, r.rootDetected, r.hookingDetected)
val statement = buildAttestationStatement(nonce, signals, System.currentTimeMillis())

// 3. Send the statement to your service; it returns a session token.

// 4. Bind each sensitive request to the attested key.
val envelope = signRequest(
    alias, sessionToken, "POST", "/secret", bodySha256,
    System.currentTimeMillis(), freshRandomNonce,
)
```

Request binding (step 4) is available on Android;
[10.4.4](#1044-binding-a-request-to-the-device) shows how your API checks it.
If you need it on iOS, raise it before you design around it.

#### 10.3.1 Flutter

Add `stringguard_flutter` to `dependencies` (see [3.5](#35-flutter)).
`StringGuardClient` runs the whole check in one call:

```dart
import 'package:stringguard_flutter/stringguard_flutter.dart';

final sg = StringGuardClient(
  baseUrl: 'https://attest.example.com',   // your attestation service
  licenseToken: licenceToken,
  tenantId: 'your-app',
);

// At startup, and again when the session expires:
final session = await sg.establishSession(deviceId: deviceId);
final token = session.sessionToken;   // null when the device was blocked
// Send `token` to your own API with every request (10.4).

// For a sensitive request (Android only; null elsewhere):
final signed = await sg.signedRequest(
  sessionToken: token!, method: 'POST', path: '/payments', body: bodyBytes,
);
```

Send `bodyBytes` as the request body exactly, and carry the values
`signedRequest` returns as in [10.4.4](#1044-binding-a-request-to-the-device):
`timestamp` as `X-Request-Timestamp`, `requestNonce` as `X-Request-Nonce` and
`signature` as `X-Request-Signature`. When it returns `null` the device cannot
sign; treat the request as unbound.

The runtime guard on its own, without attestation:

```dart
final report = await StringguardFlutter().guardReport();
if (report.suspicious) { /* your decision */ }
```

### 10.4 Deciding in your backend

```
Your app ──► attestation service ──► session token
Your app ──► your own API, with the session token on every request
Your API ──► verifies the token offline, reads the decision, decides
```

The attestation service never sits between your app and your API. It checks
the device, writes the result into a signed session token, and gives that to
the app. Your API verifies the token itself, with no call back to the service,
and decides what the request may do.

#### 10.4.1 Verifying the session token

The session token is a standard **JWT signed with Ed25519** (`alg: EdDSA`).
Verify it with the JWT library your backend already uses; you need nothing from
StringGuard on the server.

The verification keys are published by your attestation service as a JSON Web
Key Set:

```
https://<your attestation service>/.well-known/jwks.json
```

Fetch it through your library's key-set support rather than copying the keys
in: when you rotate session keys, the new key appears there and your backend
picks it up without a change. Configure the library to:

- accept **only** the `EdDSA` algorithm,
- require the issuer **`stringguard-attest`**,
- enforce the expiry (most libraries do by default),
- if more than one of your apps uses the same attestation service, require the
  `lic` claim to be the licence this API serves. Otherwise a token issued to one
  app is accepted by another app's API.

Fetch the key set over HTTPS, or over a network you trust. The attestation
service speaks plain HTTP behind your ingress, and anyone able to replace the key
set in transit could write tokens your backend accepts.

Node.js, with [`jose`](https://github.com/panva/jose):

```js
import { createRemoteJWKSet, jwtVerify } from 'jose';

const keys = createRemoteJWKSet(new URL('https://attest.example.com/.well-known/jwks.json'));

const { payload } = await jwtVerify(token, keys, {
  issuer: 'stringguard-attest',
  algorithms: ['EdDSA'],
});
```

Python, with [PyJWT](https://pyjwt.readthedocs.io/) (`pip install "pyjwt[crypto]"`):

```python
import jwt

keys = jwt.PyJWKClient("https://attest.example.com/.well-known/jwks.json")

claims = jwt.decode(
    token,
    keys.get_signing_key_from_jwt(token).key,
    algorithms=["EdDSA"],
    issuer="stringguard-attest",
)
```

Most JWT libraries support EdDSA, among them `nimbus-jose-jwt` for Java and
`golang-jwt` for Go. Check that yours does before you choose the endpoint
layout around it.

#### 10.4.2 What the token says

| Claim | Meaning |
|---|---|
| `decision` | The result of the check: `ALLOW`, `ALLOW_LIMITED` or `FLAG`. |
| `sub` | The device identifier your app supplied. **It is chosen by the app, not proven by the device**, so do not grant anything on the strength of `sub` alone. Where you need a device identity an attacker cannot choose, use `device_key`. |
| `lic` | The licence the app used. |
| `exp` | When the token expires, in seconds since the epoch. Sessions are short-lived; the app checks again for a new one. |
| `device_key` | The device's attested public key (base64, SubjectPublicKeyInfo DER). Present only when the device proved itself with hardware. Used for [request binding](#1044-binding-a-request-to-the-device). |
| `iss` | Always `stringguard-attest`. |

A blocked device is given **no token at all**, so a verified token never
carries `DENY`.

A token stays valid until `exp`. Revoking a licence or removing a device takes
effect at the app's next check, so a token issued just before remains usable for
up to the session lifetime: 15 minutes unless you set `SG_SESSION_TTL_MS`
lower.

#### 10.4.3 Deciding

What each result should allow is yours to decide; [10.5](#105-recommended-policy)
gives our recommendation. Whatever you choose, keep one
rule:

> **A request without a valid token must never be allowed more than a request
> carrying the least-trusted token.**

Your app sends the token, so an attacker can simply leave it out. If a flagged
device is refused a payment but a request with no token is allowed one, the
attack is to stop sending the token. A blocked device has no token at all, so it
arrives in exactly that position. Restrict a missing token at least as much as
you restrict `FLAG`, and require a valid token for anything sensitive.

A typical mapping:

| What your API received | Meaning | A typical response |
|---|---|---|
| No token, or one that does not verify | The device was not checked, or was blocked | Treat as an unchecked client: allow only what you would allow anyone, and never more than `FLAG`. Refuse sensitive actions. |
| `ALLOW` | The device proved it is genuine | Full access. |
| `ALLOW_LIMITED` | The device could not prove it is genuine hardware | Normal use. Usually not an attack — older devices and emulators land here. Consider a step-up for your most sensitive actions. |
| `FLAG` | The device showed signs of tampering | Allow ordinary use; refuse or step up sensitive actions such as payments. |

The decision is in the token, so you can change how you respond to it at any
time without shipping an app update.

#### 10.4.4 Binding a request to the device

A session token can be copied off the device that received it. For requests
where that matters, the app can also **sign each request with the device's
attested key**, and your API checks that the signature matches the
`device_key` in the token. A copied token is then useless without the device.
This is available on Android when the device proved itself with hardware —
that is, when the token carries `device_key`.

The app produces a signature for each request with `signRequest(...)` (see
[10.3](#103-adding-it-to-your-app)) and sends it with the request. How you
carry it is up to you; for example:

```
Authorization:       Bearer <session token>
X-Request-Timestamp: 1767225600000          (milliseconds since the epoch)
X-Request-Nonce:     <base64, a fresh random value per request>
X-Request-Signature: <base64>
```

Your API then:

1. **Verifies the session token** as in [10.4.1](#1041-verifying-the-session-token)
   and takes its `device_key`. **If the endpoint requires binding and the token
   has no `device_key`, refuse the request.** Do not treat binding as "not
   applicable" and let it through: a token without `device_key` is exactly what a
   device that could not prove itself presents, so skipping the check for it
   would let any such device past.
2. **Rejects a timestamp** more than 60 seconds from its own clock. Parse the
   header only to compare it; step 3 uses its text exactly as received.
3. **Rebuilds the signed bytes** from what it actually received — the body as
   raw bytes, before any parsing. Each of these fields, in this order, is
   written as a 4-byte big-endian length followed by its bytes:

   | # | Field |
   |---|---|
   | 1 | the text `sgreq.v1` |
   | 2 | the session token, exactly as received |
   | 3 | the HTTP method, e.g. `POST` |
   | 4 | the path including any query string, exactly as the app requested it, e.g. `/payments?id=7` |
   | 5 | the SHA-256 of the request body (32 bytes; the hash of an empty body if there is none) |
   | 6 | the timestamp as decimal text, e.g. `1767225600000` |
   | 7 | the nonce, decoded from base64 |

4. **Verifies the signature**: ECDSA on P-256 with SHA-256, DER-encoded,
   against `device_key`.
5. **Records the nonce, and rejects it if it is already recorded** — in one
   step, so two copies of a request cannot both pass. Record it only after the
   signature verifies, so a forged request cannot use up a genuine one's nonce.
   Keep accepted nonces for two minutes, **in one store shared by every
   instance, and by every API that accepts bound requests**: the signature does
   not cover the host, so with separate stores a request accepted by one could
   be replayed to another.
6. **Acts on exactly the method, path and body it verified**, not on a
   re-parsed or re-serialised form of them.

In Node.js, with only the built-in `crypto` module:

```js
import crypto from 'node:crypto';

function requestIsFromTheDevice({ token, claims, method, path, body, timestamp, nonce, signature }) {
  if (!claims.device_key) return false;
  const deviceKey = crypto.createPublicKey({
    key: Buffer.from(claims.device_key, 'base64'), format: 'der', type: 'spki',
  });
  const field = (b) => {
    const n = Buffer.alloc(4);
    n.writeUInt32BE(b.length);
    return Buffer.concat([n, b]);
  };
  const signed = Buffer.concat([
    field(Buffer.from('sgreq.v1')),
    field(Buffer.from(token)),
    field(Buffer.from(method)),
    field(Buffer.from(path)),
    field(crypto.createHash('sha256').update(body).digest()),
    field(Buffer.from(String(timestamp))),
    field(Buffer.from(nonce, 'base64')),
  ]);
  return crypto.verify('sha256', signed, deviceKey, Buffer.from(signature, 'base64'));
}
```

Check the timestamp before calling it (step 2) and record the nonce only if it
returns `true` (step 5). `body` is the request body as raw bytes, exactly as
received.

### 10.5 Recommended policy

- **Observe first.** Log the results and restrict nothing until you have seen
  what your real users get in the dashboard.
- **Never block sign-in or basic use on `ALLOW_LIMITED`.** Older devices,
  emulators and rooted phones, including your own developers', land there
  legitimately.
- **Restrict specific sensitive actions only**, such as a payment or releasing a
  key, and prefer a step-up to a refusal.
- **Change it whenever you need to.** The policy lives in your backend, so you
  can loosen or tighten it without an app update.

### 10.6 If you ship an SDK

If your SDK uses StringGuard and your customer's app uses it too:

- **Use a tenant-scoped key alias.** Pass your own tenant id to
  `deviceKeyAlias(tenantId)`. Two integrations sharing one alias overwrite each
  other's device key.
- **Do not shade or rename the StringGuard packages.** Integrations in one app
  share the device check, so a second one adds little startup cost; a shaded copy
  cannot share it and also conflicts with the runtime guard.
- **Keep your app talking to your own domain.** Hold your StringGuard licence on
  your server and expose the endpoints through your own API. Your customers'
  apps then call no third party they did not choose, and your licence never
  ships inside an application.

### 10.7 Running the service

The service runs in your infrastructure, so device traffic and the records
below never leave it. It is delivered as an archive you unpack and run on any host with Java.

#### 10.7.1 What you need

| | |
|---|---|
| Java | 17 or newer. The service tells you plainly if the runtime it finds is too old. |
| A data directory | Writable, on storage that survives a restart, and **included in your backups**. It holds your licence catalogue and the list of devices that have used your app. |
| TLS in front | The service expects to sit behind your own ingress and speaks plain HTTP to it. |
| Attestation roots | The Google hardware-attestation roots and the Apple App Attest root, downloaded as in [10.7.2](#1072-install). The service does not start without at least one. |
| Session signing keys | Generated by the service, as in [10.7.2](#1072-install), and kept in your secret store. Your backend verifies session tokens against the matching public keys. |

#### 10.7.2 Install

Download the archive and check it:

```bash
curl -fsSLO https://duranun.github.io/stringguard/service/stringguard-attest-service-1.0.0.tar
curl -fsSLO https://duranun.github.io/stringguard/service/stringguard-attest-service-1.0.0.tar.sha256
shasum -a 256 -c stringguard-attest-service-1.0.0.tar.sha256
tar -xf stringguard-attest-service-1.0.0.tar
cd stringguard-attest-service-1.0.0
```

Create the session signing keys, once, and store the printed value in your
secret store as `SG_SESSION_KEYS`:

```bash
bin/stringguard-attest keys
```

To rotate later, run it with the current value; the new key signs from then on
and the old one keeps verifying the tokens it already issued:

```bash
SG_SESSION_KEYS="<current value>" bin/stringguard-attest keys --rotate
```

Download the attestation roots for the platforms you ship:

```bash
# Android (needs jq)
curl -fsSL https://android.googleapis.com/attestation/root | jq -r '.[]' > google-roots.pem
# iOS
curl -fsSLO https://www.apple.com/certificateauthority/Apple_App_Attestation_Root_CA.pem
```

Create the operator token, also for your secret store:

```bash
openssl rand -hex 32
```

#### 10.7.3 Settings

All configuration is read from the environment.

| Setting | |
|---|---|
| `SG_PROFILE` | Set to `production`. Anything else runs permissively and reports that it is doing so. |
| `SG_LICENSE_PUBLIC_KEY` | The licence public key supplied with your licence. |
| `SG_SESSION_KEYS` | The value printed by `bin/stringguard-attest keys`. |
| `SG_ANDROID_ROOTS` | Path to the Google root certificates file; several paths are separated by `;`. PEM or DER. |
| `SG_APPLE_ROOT` | Path to the Apple App Attest root certificate. PEM or DER. |
| `SG_DATA_DIR` | The data directory. Defaults to `./data`. |
| `SG_HOST`, `SG_PORT` | Where the device API listens. Defaults to all interfaces on 8787. |
| `SG_ADMIN_HOST`, `SG_ADMIN_PORT` | Where the operator API listens. Loopback by default, and not listening at all until you set a port. |
| `SG_ADMIN_TOKEN` | The operator credential — used by both the API and the dashboard. At least 32 random characters. |
| `SG_DASHBOARD` | Set to `off` to serve the API without the dashboard. |
| `SG_AUDIT_LOG` | Path to the decision log. Omit it and no log is written. |
| `SG_SESSION_TTL_MS` | How long a session token stays valid. Defaults to 15 minutes. |
| `SG_RATE_PER_MIN` | Requests per minute per caller on the device API. Defaults to 120. |
| `SG_THREADS` | Request concurrency. Sized from the host by default. |
| `SG_ALLOW_APPLE_DEVELOPMENT` | Leave unset. Enabling it in production is refused. |

If you ship for only one platform, its roots are enough; devices on the other
platform then get `ALLOW_LIMITED`.

#### 10.7.4 Check it before you go live

```
bin/stringguard-attest doctor
```

This checks the configuration and starts nothing. It reports what is missing,
what is unsafe and what to do about each, and it exits non-zero if the
configuration is not fit to deploy. Run it as part of your install, and send us
its output if you need support — it is the fastest way for us to see a
deployment we cannot reach.

`run` performs the same check before it serves, and stops rather than starting
in a degraded state.

#### 10.7.5 Run it

```
bin/stringguard-attest run
```

**Run exactly one instance.** Two instances do not share licences, devices or
checks, and devices then fail as if at random. A second instance started on the
same data directory refuses to start. For redundancy, let your orchestrator
restart the single instance rather than running two.

Two endpoints are there for your orchestrator:

| | |
|---|---|
| `GET /healthz` | Whether the process is up. |
| `GET /readyz` | Whether it should be sent traffic. Returns 503 when it should not — for example when the data directory has become unwritable — and recovers on its own. |

Point your liveness probe at the first and your readiness probe at the second.
A restart on a failing readiness probe will not help; taking the instance out
of rotation will.

#### 10.7.6 Day-to-day operation

A licence reaches the catalogue through the operator surface, and so does every
later intervention. There are two ways in — a dashboard and an API — and they
share one listener and one credential.

**It listens on its own port**, on loopback unless you say otherwise, and only
once you set `SG_ADMIN_PORT` and `SG_ADMIN_TOKEN`. Keep it off the path your
application reaches: it creates and revokes licences.

##### The dashboard

At `/ui/` on the operator port. Sign in with `SG_ADMIN_TOKEN`; there is no
separate account to create.

It has three pages:

- **Overview** — whether anything needs attention right now: devices that
  were blocked, marked suspicious or reported a threat in the last 24 hours,
  one line per device.
- **Licences** — each licence, the app identity devices must match, and the
  devices that have used it with when each was last seen. Add, update or revoke
  a licence here, or remove a device that was lost or replaced.
- **Security events** — what happened on each device check and why, problems
  only by default. Operator changes are listed separately underneath.

Two things to know before you reach it:

- **It needs a secure context.** The session cookie is marked `Secure`, so
  signing in works over HTTPS or from `localhost` — an SSH tunnel to the
  loopback port is the simplest route — and not over plain HTTP to a remote
  address. If the sign-in form keeps returning, that is why.
- **The pages carry no JavaScript**, so the dashboard works with scripts
  disabled.

##### The API

For your own tooling — a provisioning step in your billing system, for example.
Every call carries `Authorization: Bearer $SG_ADMIN_TOKEN`.

| | |
|---|---|
| `PUT /admin/licences/{id}` | Add or update a licence: the app identity attestations must match. |
| `GET /admin/licences` | The catalogue, with how many devices have used each licence and whether it is revoked. |
| `POST /admin/licences/{id}/revoke` | Revoke. Takes effect on the next call from any device. |
| `GET /admin/licences/{id}/devices` | The devices that have used the licence. |
| `DELETE /admin/licences/{id}/devices/{deviceId}` | Remove a device and everything stored about it. This is also how you resolve a deletion request for a device. |
| `GET /admin/licences/{id}/devices/{deviceId}/keys` | The keys that device has attested with, for a rotation or incident review. |

Adding a licence looks like this:

```
curl -X PUT -H "Authorization: Bearer $SG_ADMIN_TOKEN" \
  -d '{"packageNames":["com.example.app"],
       "signingCertSha256":["<sha256 of your signing certificate>"],
       "appleAppId":"TEAMID.com.example.app"}' \
  "http://127.0.0.1:$SG_ADMIN_PORT/admin/licences/your-licence-id"
```

A licence must name your app on at least one platform.

##### Either way

Changes that alter what the service allows — creating or updating a licence,
revoking one, removing a device — are written to the decision log, with
the surface they came through. Reads are not logged.

##### Seeing what happened

The **Security events** page shows the decision log newest first: the result
of every device check and the reasons behind it. A licence's own page shows its
recent problems and links to the same view narrowed to that licence. It needs
`SG_AUDIT_LOG` set; without it the page says so rather than showing an empty
table.

Debugger, hooking and root findings appear in each check's reasons, per device
and per licence. To record one that happens later in a session, the app can
report it as it happens:

```
POST /attest/report { "sessionToken": "...", "categories": ["HOOKING_FRAMEWORK"] }
```

It uses the session token the app already holds, and appears in the log as
`CLIENT_REPORT`. **A report changes no decision**; it is there so you can see
trends across your users. One device reporting a hooking framework means
little; four hundred on one release is something to act on.

Without this service, use the `Detections` callback described in the README's
runtime guard section.

#### 10.7.7 What remains yours

| | |
|---|---|
| TLS termination, and passing the real client address | So that rate limiting counts per caller rather than per proxy. |
| Custody of the session signing private keys | A KMS or HSM rather than the environment, if your policy requires it. |
| Keeping the root certificate files current | Google and Apple publish new ones from time to time; without them, genuine devices are rejected. |
| Backups of the data directory | Losing it loses your catalogue and your device history. |
| Capacity | Size the one instance for your traffic — `SG_THREADS` and its memory limit, not more replicas. |
| Audit-log retention | Per your own policy. |

One habit worth adopting from the start: **rotate session keys by adding, not
replacing.** Add the new current key, keep the old one for verification, and
drop it once its tokens have expired. Replacing outright invalidates every
session token in flight.

For what the service processes, stores and never touches — the input to your
DPA — see [DATA_HANDLING.md](DATA_HANDLING.md).

## 11. Migrating an existing codebase

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

## 12. Troubleshooting

**`Incompatible Gradle JVM version … supports Java 1.8 and 24` (Android Studio).**
Android Studio's bundled JBR may be JDK 25, which Gradle 8.14 rejects. Set the
Gradle JDK to a version in the 17–24 range:

- *Settings → Build, Execution, Deployment → Build Tools → Gradle → Gradle JDK*, then pick or add a JDK in the 17–21 range, **or**
- set `org.gradle.java.home=/path/to/jdk21` in `gradle.properties` (and/or `java.home=…` in `.gradle/config.properties`, which is what the IDE's `#GRADLE_LOCAL_JAVA_HOME` option reads),

then run **Sync Project with Gradle Files**. Do not click *"Apply compatible
Gradle JDK configuration"* without checking what it selects, because it may pick
an unsuitably old JDK.

**`Plugin [id: 'com.stringguard'] was not found`.** The repository your licence
names is missing from `pluginManagement.repositories`
([section 2.1](#21-declaring-the-repositories)).

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

## 13. FAQ

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
