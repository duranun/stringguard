# stringguard_flutter

The Flutter plugin for StringGuard's native device-trust layer. It exposes
guard's runtime self-protection (RASP) signals and the hardware attestation
client to Dart over a method channel. String obfuscation itself is a build-time
concern handled by the `stringguard` package and its code generator, not by
this plugin.

## Requirements

| Component | Requirement |
|---|---|
| Dart SDK | `^3.5.0` |
| Flutter | `>=3.3.0` |
| Android | `minSdk` 24, `compileSdk` 36 |
| iOS | 15.0 |

The Android implementation resolves `com.stringguard:guard-jvm` and
`com.stringguard:attest-core-jvm` from the repository your licence names, which
must be reachable from your Android build. Your licence also names where to
obtain this package.

## The iOS framework

The iOS implementation is delivered as a binary `StringGuard.xcframework`, which
the podspec vendors and `Package.swift` declares as a binary target. It must be
in place at `ios/stringguard_flutter/StringGuard.xcframework` before
`pod install` or an Xcode build. Your licence names where to obtain it, and
which version pairs with which release of this package.

## API

`StringguardFlutter` is the entry point. All calls are asynchronous and
delegate to `StringguardFlutterPlatform.instance`.

| Method | Returns |
|---|---|
| `guardReport({bool rootIsSuspicious = false})` | A `GuardReport` holding `debuggerAttached`, `rootDetected`, `hookingDetected` and `suspicious`. |
| `attest({required Uint8List nonce, required String tenantId})` | An `AttestationResult`. On Android it carries the leaf-first attestation certificate chain; on iOS it carries the App Attest `keyId` and `attestationObject`, and `isAppAttest` is true. Either way, `supported: false` with a `reason` when the platform cannot attest. |
| `signRequest(...)` | The ECDSA signature binding a request to the attested key, or `null` when signing is unavailable. |
| `signDeviceProof({required String tenantId, required String audience, required Uint8List nonce})` | A proof of possession over a server nonce for one tenant, or `null` when signing is unavailable. |
| `deviceChain({required String tenantId})` | Android: the certificate chain of the key attested earlier, leaf first, or `null` when there is none. Reading it creates no key. `null` on iOS. |
| `deleteDeviceKey({required String tenantId})` | Nothing. On Android it removes the attested key for the tenant from the keystore. On iOS it forgets the remembered App Attest key id; Apple provides no API to delete the key itself, which lives for the life of the installation. |

`StringGuardClient`, exported from `src/session.dart`, wraps the challenge,
attest and verify sequence into a single `establishSession` call, renews the
session with `refreshSession` (a proof signed with the key already attested, so
no new key and, on iOS, no call to Apple), and builds the signed envelope for
each sensitive request. It holds no retry or caching logic.

## Scope and limitations

Platform support, so you can branch rather than discover it at runtime:

| Call | Android | iOS |
|---|---|---|
| `guardReport` | ✓ | ✓ |
| `attest` | ✓ | ✓ |
| `signDeviceProof` | ✓ | ✓ |
| `signRequest` | ✓ | returns `null` |
| `deviceChain` | ✓ | returns `null` |
| `deleteDeviceKey` | ✓ | no-op |

The two iOS cases report their result rather than pretending to succeed, so a
`null` is a branch to handle and not an error.

`signRequest` is Android-only: Apple's attestation does not expose the device
key for signing arbitrary payloads. If you need per-request binding on iOS,
raise it with us before you design around it.

`deleteDeviceKey` does nothing on iOS because Apple provides no API to destroy
an App Attest key; it is destroyed when the application is uninstalled.

On iOS the App Attest key id returned by `attest` is remembered, because Apple
hands it back once at creation and it is the only way to use the key again.
`signDeviceProof` returns `null` until `attest` has succeeded for that tenant,
since Apple attestation is one-shot per key.

`attest` reports `supported: false` on Android below API 26 and on emulators
without a trusted execution environment. Callers degrade to the software tier
in that case.
