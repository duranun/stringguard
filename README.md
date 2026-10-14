# StringGuard

**English** · [Türkçe](README.tr.md)

Compile-time **string obfuscation** for Kotlin — **KMP / Compose Multiplatform and
plain (non-KMP) Android / JVM** from a single Gradle plugin.

Every eligible string literal in your project is encrypted at compile time and
replaced with a call that decrypts it at runtime. The plaintext never appears in the
compiled binary, so a `strings` scan or casual decompile of the APK / jar / framework
turns up nothing.

```kotlin
val apiKey = "sk_live_51H...Z"   // you write this
// in the binary: stringGuardReveal(byteArrayOf(0x01, 0x4e, ...))  ← no plaintext
```

- **One plugin, every Kotlin target** — JVM/Desktop, Android (KMP *and* classic), iOS/Native, JS, Wasm.
- **Zero annotations to start** — obfuscate-all is on by default. You opt code *out*, not in.
- **Safe by construction** — `const val` and annotation arguments are never touched, so Retrofit / kotlinx.serialization / Room keep working.

> **Latest version:** `0.1.0` — see [Releases](../../releases).

---

## Install

**1.** Add the StringGuard Maven repository in your `settings.gradle.kts`:

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

**2.** Apply the plugin in your module (next to your Kotlin plugin — order doesn't matter):

```kotlin
plugins {
    kotlin("android")    // or kotlin("jvm") / kotlin("multiplatform")
    id("com.stringguard") version "0.1.0"
}
```

**3.** Build. That's it — the `annotations` + `runtime` dependencies are added for you,
and obfuscate-all is on by default, so you don't need to annotate anything.

➡️ **Full walkthrough:** [Integration Guide](docs/INTEGRATION.md) · [Entegrasyon Kılavuzu (TR)](docs/INTEGRATION.tr.md)

---

## Control what gets obfuscated

```kotlin
import com.stringguard.annotations.DontObfuscate

@DontObfuscate                 // keep a whole file/class/object/function readable
object PublicConstants {
    val featureFlag = "enable_new_home"
}
```
```kotlin
stringGuard {
    minLength = 4
    keepRules {
        keepPackage("com.myapp.resources.*")   // package (+ subpackages) stays plain
        keepMatching("^https://cdn\\.")         // literals matching this regex stay plain
    }
}
```

`const val` initializers, annotation arguments (`@SerialName`, Retrofit `@GET`, …) and
annotation-class bodies are **always** left plain — that's why it's safe to turn on
project-wide. Prefer to opt in instead? `stringGuard { obfuscateAll = false }` and mark
scopes with `@Obfuscate`.

---

## Honest security model

On-device obfuscation is **not cryptographic secrecy**. Decryption happens on the
device, so the key is derivable there; a determined attacker using dynamic analysis
(debugger, Frida, hooks) can still recover values. What StringGuard gives you is
**raising the cost of static analysis** — it defeats `strings`, casual decompiling and
automated secret scanners, which is what most leaks rely on. Treat it as a deterrent
and an IP-protection layer, **not** as a vault for a secret that must never leak — a
truly secret value belongs on a server, not in a client binary.

---

## Licensing

StringGuard verifies an **offline** license at build time (no network call, nothing is
phoned home) and **never blocks your build** — an empty/invalid license just falls back
to the free tier. Premium tiers (full KMP targets, RASP, advanced hardening) are on the
roadmap. To ask about a license, open an issue or get in touch.

---

## Links

- 📘 [Integration Guide](docs/INTEGRATION.md) (EN) · [Entegrasyon Kılavuzu](docs/INTEGRATION.tr.md) (TR)
- 🏷️ [Releases](../../releases)
- 🔧 Maven repo: `https://duranun.github.io/stringguard/maven`

Source is private. This page distributes the built plugin and its documentation.
