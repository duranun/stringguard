# StringGuard

**English** · [Türkçe](README.tr.md)

**String obfuscation for Kotlin apps.**

StringGuard is a Gradle plugin that transforms string constants at compile time, making it harder to extract those values from a compiled app through static analysis.

Some strings in Android apps can reveal more than you'd expect. API addresses, internal identifiers, error messages and certain details about how an app works can be found easily by inspecting an APK.

StringGuard aims to make that kind of inspection harder without changing the way you write Kotlin code.

## How it works

StringGuard transforms eligible string constants during the build. Instead of leaving the original values directly in the compiled code, it uses a different representation that is resolved at runtime.

So you keep working with normal Kotlin strings in your source while making the strings in the compiled app harder to extract statically.

## Features

- Compile-time string obfuscation
- Configurable exclusion rules
- Support for values that must be left untransformed
- Integration through Gradle
- Support for the documented Kotlin targets

## Installation

See the [integration guide](docs/INTEGRATION.md) for setup and configuration steps.

## Usage

Once configured, StringGuard runs as part of the build. You write your Kotlin code as usual:

```kotlin
private const val DEFAULT_TIMEOUT = 30

private val errorMessage = "Authentication failed"
```

Compile-time constants and unsupported or excluded values may not be transformed. See the configuration rules for details.

## Limitations

String obfuscation does not encrypt the whole app and does not mean strings can never be recovered.

A value the app needs at runtime has to be accessible to the app at some point. An attacker with enough access can still obtain these values at runtime.

StringGuard's goal is to make static analysis harder. Use it as an additional layer that supports app security, not as the only way to protect secrets embedded in an app.

## Compatibility

See the project documentation for supported Kotlin, Gradle, Android Gradle Plugin and platform versions.

## License

StringGuard is proprietary software. You may use the plugin to build and ship your own applications; the plugin itself may not be redistributed, resold or reverse engineered. See the [LICENSE](LICENSE) file for details.
