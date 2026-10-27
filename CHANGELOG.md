# Changelog

Notable changes to StringGuard. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and versions follow
[Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## 1.0.0 — 2026-10-26

### Compile-time string obfuscation

- Every eligible string literal is encrypted at compile time by a Kotlin K2
  compiler plugin and replaced with a call that decrypts it at runtime. The
  plaintext does not appear in the compiled output.
- One Gradle plugin covers Kotlin Multiplatform and Compose Multiplatform as
  well as plain Android and JVM projects, across JVM/Desktop, Android,
  iOS/Native, JS and Wasm.
- Obfuscate-all is on by default, so no annotation is needed to get a protected
  build. You opt code out rather than in, with `@DontObfuscate` / `@KeepPlain`,
  `keepPackage` and `keepMatching` rules, or a `minLength` floor.
- Literals that must remain compile-time constants — `const val` initializers
  and annotation arguments — are never touched, so Retrofit,
  kotlinx.serialization and Room keep working with obfuscation enabled
  project-wide.
- AES-GCM by default, with ChaCha20-Poly1305 available. The runtime decrypts
  both on every target.
- Test compilations are never obfuscated. Debug build types, including flavored
  ones, stay plain unless `enabledForDebug` is set.

### Hardening

- **Tamper binding (Android).** Obfuscated Android strings can be bound to your
  app signing certificates, so a repackaged, re-signed APK decrypts to junk
  rather than to your values. Up to 16 certificates, validated at configuration
  time.
- **Runtime guard (RASP).** Values inside `@GuardWith` scopes are revealed only
  when the process looks clean; under a debugger or a hooking framework a decoy
  of the same length is returned instead. Covers Android, JVM/Desktop, iOS and
  macOS.
- **Attestation (optional, self-hosted).** Server-coordinated device
  verification rooted in hardware where the platform allows it, with fail-open
  defaults. See [docs/ATTESTATION.md](docs/ATTESTATION.md).

### Other platforms

- A separate integration for applications written directly in Swift; see
  [swift/README.md](swift/README.md).
- Flutter packages for build-time obfuscated values plus a method-channel plugin
  for RASP and attestation, shipping their own privacy manifest.

### Licensing

- One feature set and one kind of licence: a valid key enables everything on
  every target. There is no free, trial or intermediate tier.
- Verification is offline, with no network call, so it works in CI and in
  air-gapped builds.
- Verification is **fatal**: a missing, malformed or expired key fails the build
  rather than producing a build that protects less than you asked for. A module
  that should not be obfuscated opts out with `enabled = false` and needs no
  key.

### Documentation

- [README](README.md) — quick start, configuration reference, security model.
- [docs/INTEGRATION.md](docs/INTEGRATION.md) — full integration walkthrough.
- [docs/ATTESTATION.md](docs/ATTESTATION.md),
  [docs/DATA_HANDLING.md](docs/DATA_HANDLING.md), [SECURITY.md](SECURITY.md).
