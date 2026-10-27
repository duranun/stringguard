# StringGuard

**English** · [Türkçe](README.tr.md)

StringGuard is a compile-time string obfuscator for Kotlin. A single Gradle
plugin covers KMP and Compose Multiplatform projects as well as plain (non-KMP)
Android and JVM projects.

By default every eligible string literal in your project is encrypted at compile
time by a Kotlin K2 compiler plugin and replaced with a call that decrypts it at
runtime. The plaintext never appears in the compiled binary, so a `strings` scan
or casual decompile of the APK, jar or framework turns up nothing.

```kotlin
val apiKey = "sk_live_51H...Z"   // you write this
```

What reaches the binary is a call that decrypts the value at runtime from an
opaque byte array. No plaintext is emitted.

- One plugin, every Kotlin target: JVM/Desktop, Android (KMP and classic),
  iOS/Native, JS, Wasm. Protection is applied the same way on every target, so
  behaviour does not vary by platform.
- No annotations are required to start. Obfuscate-all is on by default, so you
  opt code out rather than in.
- Literals that must stay compile-time constants (`const val` initializers,
  annotation arguments) are never touched, so Retrofit, kotlinx.serialization
  and Room keep working.

## Requirements

| Component | Requirement |
|---|---|
| JDK | 17+ (21 recommended). Gradle accepts Java 8 through 24; JDK 25 and 26 are rejected, see [Troubleshooting](#troubleshooting). |
| Gradle | 8.x (tested on 8.14) |
| Kotlin | 2.2.0 (the compiler plugin is pinned to the Kotlin version) |
| AGP | 8.x (for Android) |

> **Availability:** StringGuard is a licensed product. Your licence covers the
> artifacts and the repository to resolve them from; both arrive with the key.

## Installation

This is the quick path. For per-platform setup, migrating an existing codebase,
CI gates, licensing and troubleshooting, see
[docs/INTEGRATION.md](docs/INTEGRATION.md).

### Step 1: repository setup

In the consuming project's `settings.gradle.kts`:

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

The plugin adds the `annotations` and `runtime` libraries for you, which is why
both blocks need the repository; omitting either produces one of the resolution
failures under
[Troubleshooting](#troubleshooting).

### Step 2: plugin application

Apply the plugin alongside whichever Kotlin plugin your module already uses.
Order does not matter, because StringGuard wires itself to the Kotlin plugin
whenever it appears. It also adds the `annotations` and `runtime` dependencies
for you, to the right configuration or source set per project type, so there is
nothing else to declare.

<details open>
<summary>Plain Android (classic, non-KMP)</summary>

```kotlin
// app/build.gradle.kts
plugins {
    id("com.android.application")
    kotlin("android")
    id("com.stringguard") version "1.0.0"
}
```
</details>

<details>
<summary>JVM / Desktop</summary>

```kotlin
plugins {
    kotlin("jvm") version "2.2.0"
    id("com.stringguard") version "1.0.0"
}
```
</details>

<details>
<summary>Kotlin Multiplatform / Compose Multiplatform</summary>

```kotlin
plugins {
    kotlin("multiplatform") version "2.2.0"
    id("com.stringguard") version "1.0.0"
}
```
The runtime is added to `commonMain`, so every target (Android, iOS/Native, JS,
Wasm, Desktop) is covered by the same build.
</details>

### Step 3: licence key

StringGuard requires a licence key; a build without a valid one fails. Add it to
the module that applies the plugin:

```kotlin
stringGuard {
    licenseKey = "<your token>"      // or a path to a file holding it
}
```

Keep the token out of version control. Reading it from the environment is the
usual arrangement:

```kotlin
stringGuard {
    licenseKey = providers.environmentVariable("STRINGGUARD_LICENSE").getOrElse("")
}
```

See [Licensing](#licensing) for what happens when the key is missing or expired.

### Step 4: build

Obfuscate-all is on by default, so no annotation is needed to get a protected
build.

```bash
./gradlew assembleRelease   # Android
./gradlew build             # JVM / KMP
```

Every string literal is encrypted in the output except the ones listed under
[What is never obfuscated](#what-is-never-obfuscated) and anything you opt out.

## Documentation

| Document | Contents |
|---|---|
| [docs/INTEGRATION.md](docs/INTEGRATION.md) | Full integration walkthrough: per-platform setup, migrating an existing codebase, CI gates, licensing, troubleshooting. |
| [docs/ATTESTATION.md](docs/ATTESTATION.md) | Attestation: what it proves, how to integrate it, and how to set policy. |
| [docs/DATA_HANDLING.md](docs/DATA_HANDLING.md) | What the attestation service processes, retains and never touches. Input to a DPA. |
| [swift/README.md](swift/README.md) | String obfuscation for applications written directly in Swift. |
| [SECURITY.md](SECURITY.md) | How to report a vulnerability. |

## Security model and limitations

On-device obfuscation is not cryptographic secrecy. Decryption occurs on the
device, so the key is derivable there. An attacker using dynamic analysis
tooling such as a debugger, Frida or runtime hooks can recover protected values
at runtime.

What StringGuard provides is a higher cost for static analysis. It defeats
`strings`, casual decompiling and automated secret scanners, which is what
breaches most commonly rely on. This is the same shape as commercial tools such
as DexGuard: static obfuscation, plus an optional runtime self-protection (RASP)
layer and app-signature binding, both described below. No decryption key is
stored in the binary as a readable value.

Treat StringGuard as a deterrent and an IP-protection layer, not as a vault for
a secret that must never leak. A value that must never be disclosed belongs on a
server, not in a client binary.

Vulnerability reports go through [SECURITY.md](SECURITY.md).

## Controlling what gets obfuscated

### Default: everything

With no configuration, every `String` literal of length at least `minLength` is
obfuscated, project-wide. You write rules only to exclude things.

### Scope exclusion with `@DontObfuscate`

Mark a file, class, object, function, property or constructor to leave its
literals plain. This is useful for reflection keys, resource names, log tags, or
anything a tool reads by its literal value.

```kotlin
import com.stringguard.annotations.DontObfuscate

@DontObfuscate
object PublicConstants {
    val featureFlagKey = "enable_new_home"   // stays readable
}
```

`@KeepPlain` is an exact alias of `@DontObfuscate`. Use whichever reads better.

### Pattern-based exclusions with `keepRules`

```kotlin
// build.gradle.kts
stringGuard {
    minLength = 4                       // skip very short literals (default 1)
    keepRules {
        keepPackage("com.myapp.resources.*")   // this package + subpackages stay plain
        keepMatching("^https://cdn\\.")        // literals matching this regex stay plain
    }
}
```

### What is never obfuscated

The following are skipped automatically because they have to be compile-time
constants, and obfuscating them would break compilation:

- `const val` initializers, for example `const val TAG = "MyScreen"`
- Annotation arguments, for example `@SerialName("user_id")`,
  `@GET("users/{id}")`, `@Deprecated("…")`
- Annotation class bodies, meaning their default values

This is why StringGuard is safe to turn on project-wide: Retrofit endpoints,
`@Serializable` field names and similar values keep working untouched.

### Opt-in mode

To obfuscate only what you mark, turn obfuscate-all off and use `@Obfuscate` to
opt specific scopes in:

```kotlin
stringGuard { obfuscateAll = false }
```
```kotlin
import com.stringguard.annotations.Obfuscate

@Obfuscate
object Secrets {
    val apiKey = "sk_live_..."    // obfuscated; everything else in the project stays plain
}
```

## Configuration reference

```kotlin
stringGuard {
    enabled = true                 // master switch; false = plugin is a no-op
    obfuscateAll = true            // true (default): whole project; false: only @Obfuscate scopes
    minLength = 1                  // literals shorter than this stay plain
    enabledForDebug = false        // obfuscate debug build types too (off = faster/greppable dev builds)
    licenseKey = "<token>"         // REQUIRED: license text, or a path to a license file

    guard = false                  // runtime self-protection at @GuardWith reveal sites
    tamperBinding = false          // Android: strings decrypt only in an APK signed by...
    signingCertSha256("AB:CD:...") // ...one of these certificates (repeatable)

    // AEAD primitive (default AES_GCM). ChaCha20-Poly1305 is also supported:
    algorithm = StringGuardAlgorithm.CHACHA20_POLY1305
    keepRules {
        keepPackage("com.app.res.*")
        keepMatching("^android\\.")
    }
}
```

| Option | Default | Effect |
|---|---|---|
| `enabled` | `true` | Turn the whole plugin off when false. |
| `obfuscateAll` | `true` | Obfuscate the whole project, or only `@Obfuscate` scopes. |
| `minLength` | `1` | Skip literals shorter than N characters. |
| `enabledForDebug` | `false` | By default debug builds are left plain for fast, greppable development. Test compilations are always skipped. |
| `licenseKey` | `""` | **Required.** Offline license: the license text, or a path to a file holding it (relative paths resolve against the module directory). Verified with an embedded public key, no network. A missing, malformed or expired key **fails the build**. See [Licensing](#licensing). |
| `guard` | `false` | Weave runtime self-protection into reveal sites inside `@GuardWith` scopes. See [Runtime guard](#runtime-guard-rasp). |
| `tamperBinding` | `false` | Android only. Bind Android strings to your signing certificates. See [Tamper binding](#tamper-binding-android). |
| `signingCertSha256(...)` | none | Allowed signing-certificate SHA-256 fingerprints for `tamperBinding`. Required when it is on. |
| `algorithm` | `AES_GCM` | AEAD primitive: `AES_GCM` (default) or `CHACHA20_POLY1305`. The runtime decrypts both on every target. |

### Debug and release builds

Test code is never obfuscated. Debug build types are left plain unless
`enabledForDebug = true`, which keeps the inner loop fast and logs greppable.
Release builds are obfuscated.

## Licensing

StringGuard has one feature set and one kind of licence. A valid key enables
everything described in this README on every target: there is no free, trial or
intermediate tier, and nothing is withheld from a licensed build.

Verification is offline. The key is checked against a public key embedded in the
plugin, at configuration time, with no network call, so it works in CI and in
air-gapped builds.

**A build with no valid licence fails.** A missing, malformed or expired key
stops configuration with an explanatory error:

```
> StringGuard requires a license key. Set stringGuard.licenseKey to your license
  token, or to the path of a file holding it. To build this module without
  StringGuard instead, set stringGuard.enabled = false.
```

That is deliberate. The alternative — carry on and obfuscate less — turns a
green build into a shipped binary with readable secrets in it, which is the one
outcome a string obfuscator must never produce. A module that genuinely should
not be obfuscated opts out with `stringGuard { enabled = false }`, and then
needs no key.

A successful build logs the licence once:

```
StringGuard: license verified (holder=Acme Inc, expires=2027-04-01)
```

## Tamper binding (Android)

Tamper binding ties every obfuscated Android string to your app's signing
certificate. Inside an APK signed by one of the listed certificates, strings
decrypt normally. If someone re-signs your APK with their own key, which is the
usual repackaging step, every bound string decrypts to plausible junk instead.
The app does not crash, so the check is not pointed out to the attacker.

```kotlin
stringGuard {
    tamperBinding = true
    signingCertSha256("AB:CD:EF:...")   // Play App Signing key: Play Console → Test and release → App integrity
    signingCertSha256("12:34:56:...")   // upload or debug key, if builds signed with it must run
}
```

Get a fingerprint with any of:

```bash
apksigner verify --print-certs app-release.apk   # "certificate SHA-256 digest"
keytool -list -v -keystore release.jks           # "SHA256:"
```

- With Play App Signing, the APK users install is signed by Google's key, not
  your upload key. List the Play App Signing certificate, or production installs
  get junk.
- The build fails at configuration if `tamperBinding = true` has no certificates
  or a malformed one, so a misconfigured app cannot ship by accident.
- Only Android compilations are bound. iOS, Desktop, JS and Wasm targets in the
  same project are unaffected.
- Debug builds are not obfuscated by default. With `enabledForDebug = true`,
  list the debug certificate too.
- A fingerprint is **64 hex digits, optionally colon-separated**, and you may
  list at most **16 certificates**. A malformed or missing value fails the build
  at configuration time.
- Release-variant JVM unit tests run off-device, where there is no APK to read.
  The plugin keeps bound strings readable in those tests so your suite still
  works; on a real device only the installed APK's certificate counts.
- **Rotating a certificate is a staged change.** Add the new fingerprint and
  ship that build first. Remove the old fingerprint only once no installed copy
  still bound to it needs to work — removing it early degrades every one of
  those installs.
- No special Android permission or API is required, and no `Context` is needed.
- Limits: this stops re-signing without code changes. An attacker who patches
  the code can defeat it, because your certificate's fingerprint is public in
  your own APK.

## Runtime guard (RASP)

```kotlin
stringGuard { guard = true }   // adds the guard runtime to the module
```
```kotlin
import com.stringguard.annotations.GuardWith

@GuardWith
object PaymentKeys {
    val merchantKey = "mk_live_..."
}
```

Inside a `@GuardWith` scope, a protected value is revealed only when the
process looks clean. When a debugger or a hooking framework is present the real
value is never decrypted: a plausible decoy of the same length is returned
instead, so the app keeps running and the check is not advertised to whoever is
probing it.

What is detected, per platform:

| Signal | Android | JVM / Desktop | iOS | macOS | JS / Wasm |
|---|---|---|---|---|---|
| Debugger attached | ✓ | ✓ | ✓ | ✓ | — |
| Hooking framework | ✓ | ✓ | ✓ | ✓ | — |
| Root / jailbreak | ✓ | ✓ (Linux) | ✓ (device only) | — | — |

Behaviour you can rely on, and the decisions it leaves you:

- **A rooted or jailbroken device is reported but does not degrade values**, because
  many legitimate users root their phones. To refuse them instead, set
  `Guard.rootIsSuspicious = true` at startup. That is a product decision about
  turning away real users; make it deliberately.
- **Once a debugger or hook is seen, values stay degraded for the life of the
  process**, even if the tool detaches.
- **Detections are reported to you, not to us.** `Detections.onDetection` is
  called the first time each signal category fires, and `Detections.all()`
  lists everything seen so far — read that at startup as well, since a signal
  can fire before your callback is installed. StringGuard makes no network call
  and collects nothing; your own telemetry decides where the record goes.

  ```kotlin
  Detections.onDetection = { category ->
      analytics.log("stringguard.detection", mapOf("category" to category.name))
  }
  Detections.all().forEach { /* anything that fired during startup */ }
  ```

  An event carries a category — `DEBUGGER`, `HOOKING_FRAMEWORK`,
  `ROOT_OR_JAILBREAK` — and never the file or library that matched. Naming it
  would write the detector's own strings back in plaintext, into a log readable
  on a device the attacker controls. The callback runs on whichever thread saw
  the signal, so return from it immediately; anything it throws is swallowed
  rather than allowed to fail a reveal.

  Root is reported even when it does not degrade values, so you can see how many
  of your users have a rooted device without treating them as hostile. Collect
  before you enforce anything: one device reporting a hooking framework means
  little, four hundred on one release is something to act on.
- **Detection is per-process.** A check observes only the process that makes
  it. If your app reveals secrets in another process (`android:process=`, an
  isolated service, a separate native process), the `@GuardWith` scopes and any
  `Guard.report()` call must live in that process too — a check in your main
  process says nothing about the others. `Guard.rootIsSuspicious` is also
  per-process, so set it in each one. Attestation is unaffected: it binds the
  whole app on the device rather than a single process.
- **Checks are cached briefly**, so many protected strings on one screen cost a
  single probe.
- **Limits.** Every RASP check can itself be defeated by a determined attacker
  with full control of the device. This raises the cost of dynamic analysis; it
  is not a guarantee. Treat it as defence in depth behind a server-side secret,
  not as a substitute for one.

### R8 and minification

StringGuard artifacts ship their own consumer ProGuard rules, so a minifying
Android app needs no manual setup: R8 picks them up automatically. Obfuscated
strings reveal correctly through a minified build, and R8 is free to rename
everything that is not part of the public API, so minification makes the shipped
dex harder to read rather than easier.

R8 and StringGuard are complementary and intended to be used together: R8
renames and shrinks, StringGuard encrypts the string contents R8 leaves
readable.

## Verifying that no plaintext remains

Protected literals do not appear in the compiled output, and no StringGuard
release ships unless that is demonstrated on every supported backend. On JVM
and Desktop you can see it directly:

```bash
./gradlew build
javap -c -p -classpath build/libs/<your>.jar com.example.Config
# the secret literals are gone
```

## Troubleshooting

**"Incompatible Gradle JVM version … supports Java 1.8 and 24" (Android
Studio).** Android Studio's bundled JBR may be JDK 25, which Gradle 8.14
rejects. Point the Gradle JDK at a JDK between 17 and 24 under
`Settings → Build, Execution, Deployment → Build Tools → Gradle → Gradle JDK`,
or set `org.gradle.java.home=/path/to/jdk21` in `gradle.properties`, then run
*Sync Project with Gradle Files*.

**"Plugin `com.stringguard` not found."** The repository your licence names is
missing from `pluginManagement.repositories`, or its credentials are not
configured. See [Step 1](#step-1-repository-setup).

**"Could not find com.stringguard:runtime" / ":annotations".** The same
repository is missing from `dependencyResolutionManagement.repositories`. Both
blocks need it.

**A literal I expected to be encrypted is still plaintext.** It is probably a
`const val`, an annotation argument, below `minLength`, in a
`@DontObfuscate`/`@KeepPlain` scope, matched by a `keepRules` pattern, or in a
debug build with `enabledForDebug = false`.

**Retrofit / kotlinx.serialization annotation values changed?** They do not
change. Annotation arguments are never obfuscated by design.

**With `tamperBinding`, the installed app shows garbled strings.** The installed
APK is signed by a certificate you did not list. With Play App Signing that is
Google's key: add its fingerprint from Play Console → App integrity. Check the
APK with `apksigner verify --print-certs`.

**"tamperBinding = true requires at least one signingCertSha256".** Add the
fingerprint or fingerprints as shown in
[Tamper binding](#tamper-binding-android).

**"StringGuard requires a license key."** `stringGuard.licenseKey` is unset or
resolves to an empty file. Set it to your licence token, or to a path to a file
holding it (relative paths resolve against the module directory). A module that
should not be obfuscated at all sets `stringGuard { enabled = false }` instead,
which needs no key.

**"StringGuard license is not valid: ..."** The key is not a StringGuard licence
token, is truncated, or was not issued for this build of StringGuard. The reason
in the message says which. A `license format v1 is no longer supported` reason
means the key is in a superseded format; ask for a reissue.

**"StringGuard license '...' expired on ...".** Renew the licence and update
`stringGuard.licenseKey`. Builds are not allowed to continue unprotected, so
there is no override.

## Attestation (optional, advanced)

Attestation is a server-coordinated layer that answers whether an install is a
genuine, paying one on a sound device. It is stronger than client-only RASP
because the decision is made server-side from signed material the app cannot
forge.

- **Hardware-rooted where the platform allows it.** On Android it uses the
  device's hardware-backed key attestation; on iOS, Apple App Attest. Neither
  path needs Play Integrity, a Google API or a request quota, and verification
  is offline.
- **Request binding.** Sensitive requests are signed by the attested device key,
  so a relayed session token is useless without the device it was issued to.
- **Fail-open by default.** Attestation labels a session's assurance level; the
  shipped default is allow-and-report, never a blanket block. You decide what a
  low assurance level costs, per operation.
- **Self-hosted.** The verification service runs in your own infrastructure, so
  device traffic never leaves it. It is delivered as a package you unpack and
  run, with a configuration check to run before you go live, and a dashboard
  and an API for provisioning licences, managing devices and revoking.
- iOS App Attest requires the App Attest capability on the app's provisioning
  profile, and a physical device.

For what it proves, what it does not, and how to integrate and operate it, see
[docs/ATTESTATION.md](docs/ATTESTATION.md).

## License

StringGuard is commercial, proprietary software. It is not open source, and no
source license is granted.

Building with StringGuard requires a commercial licence key. There is one
feature set and no free tier. With a valid key you may build for any purpose
including commercial use, ship the runtime inside your own application and
distribute that application through app stores and to your customers. Runtime
components already shipped in a version of your application stay licensed for
your end users after your key expires. The artifacts may be redistributed
verbatim, for example by package registries and their mirrors.

The full terms are in [LICENSE](LICENSE). Third-party components and their
licenses are listed in [NOTICE](NOTICE).
