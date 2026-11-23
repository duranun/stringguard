## 1.1.0

* `StringGuardClient.refreshSession` renews a session with a proof signed by
  the key already attested. It creates no new key and, on iOS, makes no call to
  Apple. It attests again by itself when the device has no key yet or the
  service does not accept the proof.
* `deviceChain` returns the certificate chain of the key attested earlier
  (Android; `null` on iOS).
* Android handles calls off the UI thread, so key generation no longer blocks
  it.
* The iOS jailbreak check no longer uses a required-reason API; embedding the
  plugin adds nothing to the app's privacy manifest.

## 1.0.0

* Exposes guard's RASP signals (`guardReport`) and the
  hardware attestation client (`attest`, `signRequest`, `signDeviceProof`,
  `deleteDeviceKey`) to Dart over a method channel, together with the
  `StringGuardClient` session helper.
* Android and iOS are both implemented for `guardReport`, `attest` and
  `signDeviceProof`. On iOS `signRequest` returns null, because App Attest
  cannot sign an arbitrary payload, and `deleteDeviceKey` cannot delete the key,
  because Apple exposes no API for it.
