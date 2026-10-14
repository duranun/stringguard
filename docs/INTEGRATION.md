# Integration Guide

**English** · [Türkçe](INTEGRATION.tr.md)

This document describes the steps needed to add StringGuard to a project, configure it and verify the result.

## Requirements

StringGuard works with JDK versions between 17 and 24, Gradle 8.x and Kotlin 2.2.0. Android projects are expected to use Android Gradle Plugin 8.x.

Because the plugin is compiled against a specific Kotlin version, your project's Kotlin version needs to match the one above.

## Installation

Add the Maven repository where StringGuard is published to both repository blocks in your `settings.gradle.kts`. The first block resolves the plugin itself, the second resolves the `annotations` and `runtime` dependencies the plugin adds.

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

The repository is public, so no credentials are required.

## Applying the plugin

Add the plugin next to the Kotlin plugin your module uses. The `annotations` and `runtime` dependencies are added to the relevant configuration automatically.

For Android projects:

```kotlin
plugins {
    id("com.android.application")
    kotlin("android")
    id("com.stringguard") version "0.1.0"
}

stringGuard {
    enabled = true
    minLength = 4
}
```

For JVM or Desktop projects:

```kotlin
plugins {
    kotlin("jvm") version "2.2.0"
    id("com.stringguard") version "0.1.0"
}

stringGuard {
    enabled = true
    minLength = 4
}
```

For Kotlin Multiplatform and Compose Multiplatform projects:

```kotlin
plugins {
    kotlin("multiplatform") version "2.2.0"
    id("com.stringguard") version "0.1.0"
}

stringGuard {
    enabled = true
    minLength = 4
}
```

In multiplatform projects the dependencies are added to the `commonMain` source set, and all documented targets are covered by the same configuration.

## Exclusion rules

StringGuard transforms every eligible string by default. You can keep values out of that scope when they should not be transformed.

To leave a file, class or object untransformed, use the `@DontObfuscate` annotation:

```kotlin
import com.stringguard.annotations.DontObfuscate

@DontObfuscate
object PublicConstants {
    val featureFlag = "enable_new_home"
}
```

To exclude values by pattern, use the `keepRules` block:

```kotlin
stringGuard {
    minLength = 4
    keepRules {
        keepPackage("com.myapp.resources.*")
        keepMatching("^https://cdn\\.")
    }
}
```

Compile-time constants and annotation arguments are never transformed, because the language requires them to be constant values.

If you prefer to transform only specific places instead of the whole project, turn off `obfuscateAll` and mark the relevant parts with `@Obfuscate`:

```kotlin
stringGuard { obfuscateAll = false }
```

```kotlin
import com.stringguard.annotations.Obfuscate

@Obfuscate
class PaymentClient {
    private val merchantSecret = "msk_live_..."
}
```

## Configuration options

| Option | Default | Description |
|---|---|---|
| `enabled` | `true` | Enables or disables the plugin. |
| `obfuscateAll` | `true` | Whether the whole project is transformed or only the places marked with `@Obfuscate`. |
| `minLength` | `1` | Strings shorter than this are not transformed. |
| `enabledForDebug` | `false` | Also transforms debug builds. Test sources are out of scope either way. |
| `licenseKey` | `""` | Offline license key. When empty or invalid, the free tier applies. |

## Verifying

On JVM or Desktop output you can inspect whether strings were transformed with `javap`:

```bash
./gradlew build
javap -c -p -classpath build/libs/<jar>.jar com.example.Config
```

On Android you can scan the `dex` files inside the APK:

```bash
./gradlew assembleRelease
unzip -o app/build/outputs/apk/release/*-release*.apk -d /tmp/apk >/dev/null
strings /tmp/apk/classes*.dex | grep -F "expected_value"
```

On Kotlin/Native output strings are stored as UTF-16LE, so you need to scan both encodings:

```bash
strings -e l <binary> | grep -F expected_value
strings       <binary> | grep -F expected_value
```

## Troubleshooting

If Android Studio reports "Incompatible Gradle JVM version", set the Gradle JDK to a version between 17 and 24. You can find this setting under Settings > Build, Execution, Deployment > Build Tools > Gradle, or add `org.gradle.java.home` to your `gradle.properties`.

A `Plugin com.stringguard was not found` error means the Maven repository has not been added to the `pluginManagement` block.

A `Could not find com.stringguard:runtime` or similar dependency error means the repository also needs to be added to the `dependencyResolutionManagement` block.
