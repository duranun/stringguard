# StringGuard — Integration Guide

**English** · [Türkçe](INTEGRATION.tr.md)

A step-by-step guide to adding StringGuard to a real project. For the short version see
the [README](../README.md); this is the full walkthrough.

**Contents**
1. [Requirements](#1-requirements)
2. [Install](#2-install)
3. [Apply it to your module](#3-apply-it-to-your-module)
4. [First build & sanity check](#4-first-build--sanity-check)
5. [Tuning what gets obfuscated](#5-tuning-what-gets-obfuscated)
6. [Debug, release & CI](#6-debug-release--ci)
7. [Verifying the binary](#7-verifying-the-binary)
8. [Migrating an existing codebase](#8-migrating-an-existing-codebase)
9. [Troubleshooting](#9-troubleshooting)
10. [FAQ](#10-faq)

---

## 1. Requirements

| Tool | Version |
|---|---|
| JDK | 17–24 (21 recommended). **Not 25/26** — Gradle 8.14 rejects Java > 24. |
| Gradle | 8.x (tested on 8.14) |
| Kotlin | **2.2.0** — must match the plugin's compiler build |
| AGP | 8.x (Android) |

The compiler plugin is tied to the Kotlin version. If your project is not on Kotlin
2.2.0, align it first.

**Mental model:** obfuscate-all is **on by default**. You do not opt literals in; you opt
them *out*. Literals that must be compile-time constants (`const val`, annotation
arguments) are never touched, so turning it on project-wide is safe.

---

## 2. Install

Add the StringGuard Maven repository to **both** repository blocks of your
`settings.gradle.kts` — `pluginManagement` resolves the plugin, and
`dependencyResolutionManagement` resolves the `annotations` + `runtime` libraries the
plugin adds for you:

```kotlin
pluginManagement {
    repositories {
        maven("https://duranun.github.io/stringguard/maven")
        gradlePluginPortal()
        google()
        mavenCentral()
    }
}
dependencyResolutionManagement {
    repositories {
        maven("https://duranun.github.io/stringguard/maven")
        google()
        mavenCentral()
    }
}
```

No credentials are needed — the repository is public and read-only.

---

## 3. Apply it to your module

Apply StringGuard next to whichever Kotlin plugin your module already uses. **Order does
not matter.** You do **not** add `annotations`/`runtime` manually; the plugin adds them
to the correct configuration/source set.

### 3.1 Plain Android (classic, non-KMP)

```kotlin
// app/build.gradle.kts
plugins {
    id("com.android.application")      // or com.android.library
    kotlin("android")
    id("com.stringguard") version "0.1.0"
}

stringGuard {
    enabled = true
    minLength = 4
}
```
> Dependencies are added to the `implementation` configuration.

### 3.2 JVM / Desktop

```kotlin
plugins {
    kotlin("jvm") version "2.2.0"
    id("com.stringguard") version "0.1.0"
}

stringGuard { enabled = true; minLength = 4 }
```

### 3.3 Kotlin Multiplatform / Compose Multiplatform

```kotlin
plugins {
    kotlin("multiplatform") version "2.2.0"
    id("com.stringguard") version "0.1.0"
}

stringGuard { enabled = true; minLength = 4 }
```
The `annotations` + `runtime` dependencies are added to `commonMain`, so **every** target
(Android, iOS/Native, JS, Wasm, Desktop) is covered by the same configuration.

---

## 4. First build & sanity check

```bash
./gradlew assembleRelease   # Android
./gradlew build             # JVM / KMP
```

Your code is unchanged — you write ordinary string literals:

```kotlin
object Config {
    val apiBaseUrl = "https://api.example.com/v2"   // obfuscated in the release binary
    val analyticsKey = "ak_prod_1a2b3c4d5e"         // obfuscated
}
```

At runtime the values are identical to what you wrote; in the binary they are gone.
Confirm with [section 7](#7-verifying-the-binary).

---

## 5. Tuning what gets obfuscated

### Keep a whole scope readable — `@DontObfuscate`

```kotlin
import com.stringguard.annotations.DontObfuscate

@DontObfuscate                        // file, class, object, function, property or constructor
object ReflectionKeys {
    val lookupName = "com.example.Thing"   // used by Class.forName → keep readable
}
```
`@KeepPlain` is an exact alias — identical behavior, pick whichever reads better.

### Pattern-based exclusions — `keepRules`

```kotlin
stringGuard {
    minLength = 4
    keepRules {
        keepPackage("com.example.resources.*")   // package + all subpackages stay plain
        keepMatching("^https://cdn\\.")           // literals matching regex stay plain
    }
}
```

- `keepPackage("a.b.*")` keeps package `a.b` **and** every subpackage.
- `keepMatching(regex)` keeps any literal whose content matches (uses `containsMatchIn`).

### Never obfuscated automatically

Skipped automatically because they *must* be compile-time constants — obfuscating them
would not compile:

- `const val` initializers
- Annotation arguments (`@SerialName("…")`, `@GET("…")`, `@Deprecated("…")`, …)
- Annotation class default values

This is why Retrofit, kotlinx.serialization and Room keep working with obfuscate-all on.

### Opt-in mode (annotate instead of exclude)

```kotlin
stringGuard { obfuscateAll = false }
```
```kotlin
import com.stringguard.annotations.Obfuscate

@Obfuscate
class PaymentClient {
    private val merchantSecret = "msk_live_..."   // obfuscated; everything else stays plain
}
```

---

## 6. Debug, release & CI

- **Test** compilations are **always** skipped.
- **Debug** build types are left plain unless `enabledForDebug = true`.
- **Release** is obfuscated.

```kotlin
stringGuard { enabledForDebug = false }   // keep dev builds fast and logs greppable
```

**Full DSL:**

| Option | Default | Effect |
|---|---|---|
| `enabled` | `true` | Turn the whole plugin off when false. |
| `obfuscateAll` | `true` | Whole project vs. only `@Obfuscate` scopes. |
| `minLength` | `1` | Skip literals shorter than N characters. |
| `enabledForDebug` | `false` | Obfuscate debug build types too. Test code is always skipped. |
| `licenseKey` | `""` | Offline license key/path. Invalid/empty → free tier; **build is never blocked**. |
| `algorithm` | `AES_GCM` | *Reserved* — AES-GCM is always used today. |
| `tamperBinding` | `false` | *Reserved (roadmap)* — signature-bound key. |

CI needs nothing special — just run your normal release build. If CI uses a JDK 25/26
image, pin a 17–24 JDK. Consider adding the plaintext-leak check below as a build gate.

---

## 7. Verifying the binary

**JVM / Desktop:**
```bash
./gradlew build
javap -c -p -classpath build/libs/<your>.jar com.example.Config
# secret literals are gone; calls go through stringGuardReveal([B)
```

**Android:**
```bash
./gradlew assembleRelease
unzip -o app/build/outputs/apk/release/*-release*.apk -d /tmp/apk >/dev/null
strings /tmp/apk/classes*.dex | grep -F "ak_prod_1a2b3c4d5e" && echo "LEAK!" || echo "clean"
```

**Kotlin/Native (iOS backend):** Kotlin/Native stores strings as **UTF-16LE**, so scan
both encodings or you get false "clean" results:
```bash
strings -e l <binary> | grep -F your_secret   # UTF-16LE (the important one)
strings       <binary> | grep -F your_secret   # UTF-8
```

---

## 8. Migrating an existing codebase

1. **Apply the plugin** ([section 3](#3-apply-it-to-your-module)) and build once. Because
   obfuscate-all is the default, everything eligible is obfuscated immediately — no code changes.
2. **Build release and run your tests / smoke tests.** The guardrails cover `const val`
   and annotation args; if your app reads a *runtime* string literal by identity somewhere
   unusual (reflection on a literal, a hand-rolled resource lookup keyed by a literal),
   exclude it with `@DontObfuscate` or a `keepRules` entry.
3. **Raise `minLength`** (e.g. `4`) to skip trivial literals and shrink output.
4. **Add the plaintext-leak check** ([section 7](#7-verifying-the-binary)) to CI.
5. Prefer the **opt-in** model? Set `obfuscateAll = false` and annotate sensitive scopes
   with `@Obfuscate`.

**Safe, no action needed:** Retrofit/Ktor endpoint annotations, `@SerialName` field names,
`const val` log tags, DI qualifiers using annotation arguments.

---

## 9. Troubleshooting

**`Incompatible Gradle JVM version … supports Java 1.8 and 24` (Android Studio).**
Android Studio's bundled JBR may be JDK 25, which Gradle 8.14 rejects. Point the Gradle JDK
at a JDK 17–24: *Settings → Build, Execution, Deployment → Build Tools → Gradle → Gradle JDK*,
or set `org.gradle.java.home=/path/to/jdk21` in `gradle.properties`, then *Sync Project with
Gradle Files*.

**`Plugin [id: 'com.stringguard'] was not found`.** The StringGuard Maven repository is
missing from `pluginManagement.repositories` (or the URL is wrong). See [section 2](#2-install).

**`Could not find com.stringguard:runtime` / `:annotations`.** Add the StringGuard Maven
repository to `dependencyResolutionManagement.repositories` too.

**A literal I expected encrypted is still plaintext.** Check: it is a `const val`; an
annotation argument; shorter than `minLength`; in a `@DontObfuscate`/`@KeepPlain` scope;
matched by a `keepRules` pattern; or in a debug build with `enabledForDebug = false`.

**A string I needed readable at runtime (reflection) got obfuscated.** Exclude that scope
with `@DontObfuscate` or a `keepRules` entry.

---

## 10. FAQ

**Do I have to annotate anything?** No. Obfuscate-all is the default; annotations are only
for exclusions (`@DontObfuscate`/`@KeepPlain`) or the optional opt-in mode (`@Obfuscate`).

**Does it work without R8/ProGuard?** Yes, and alongside them — they are complementary
(R8 renames/shrinks, StringGuard encrypts string contents).

**Does it slow the app down?** Decryption is per-literal, tiny and synchronous; negligible
for typical secret/config strings. Use `minLength`/`keepRules` to skip huge or hot-loop literals.

**Is this real security?** It raises the cost of static analysis and defeats casual
`strings`/decompile and automated scanners. It is **not** cryptographic secrecy — the key is
derivable on-device. A must-never-leak secret belongs on a server.

**iOS and Wasm too?** Yes — the runtime is pure-Kotlin in `commonMain`; the same encrypted
payload decrypts identically on JVM, Kotlin/Native (iOS backend), JS and Wasm.

**Can I change the cipher to ChaCha20 / tamper-bind the key?** Not yet — `algorithm` and
`tamperBinding` are accepted by the DSL but reserved; AES-GCM is always used today.
