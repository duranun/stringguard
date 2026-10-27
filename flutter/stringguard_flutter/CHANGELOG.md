## 1.0.0

* Exposes guard's RASP signals (`guardReport`) and the
  hardware attestation client (`attest`, `signRequest`, `signDeviceProof`,
  `deleteDeviceKey`) to Dart over a method channel, together with the
  `StringGuardClient` session helper.
* Android and iOS are both implemented for `guardReport`, `attest` and
  `signDeviceProof`. On iOS `signRequest` returns null, because App Attest
  cannot sign an arbitrary payload, and `deleteDeviceKey` cannot delete the key,
  because Apple exposes no API for it.
