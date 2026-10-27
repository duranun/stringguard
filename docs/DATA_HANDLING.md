# Data handling and privacy

This document records what the StringGuard attestation and licence service
processes, why, for how long, and what it does not touch. It is the reference a
security or privacy reviewer uses to complete a DPA and a vendor assessment.
Every data category below corresponds to a concrete field the service handles.

> **The service is self-hosted.** It runs in your own infrastructure, so the
> data described here stays there and never reaches StringGuard. That is why a
> self-hosting customer has no StringGuard subprocessors.

## Data processing in the obfuscation path

The core product, meaning compile-time string obfuscation (compiler plugin,
Gradle plugin, runtime, `annotations`) and the on-device `guard` RASP, runs
entirely on the developer's build machine and the end-user's device. It makes no
network calls and sends nothing to StringGuard. A customer can use the
obfuscation layer with no data leaving their control. Only the optional
attestation and licensing feature contacts a backend, and only that feature is
covered below.

## Data categories processed by the backend

| Category | Field(s) | Source | Stored? | Purpose |
| --- | --- | --- | --- | --- |
| License identifier | `licenseId`, `licenseToken` | Issued by StringGuard to the customer | License token verified transiently; `licenseId` stored in catalog + audit | Identify the commercial license |
| Device identifier | `integrity.deviceId` | Client-supplied, opaque | Stored (device-key map, audit) | **Pseudonymous**: tells devices apart in the device list and the decision log; not a security boundary |
| App identity | `packageName`, `signingCertSha256` | Read from the signed attestation | Compared transiently; not persisted as such | Bind the session to the licensed app |
| Attested device key | attestation leaf public key | Android hardware attestation chain | Leaf **public** key stored per license+device | Request binding + key rotation audit |
| Attestation chain | `attestationChainB64` (DER certs) | Device KeyStore | **Not persisted**: verified transiently, then discarded | Prove hardware-backed key |
| Integrity signals | `debuggerAttached`, `rootDetected`, `hookingDetected`, `ts` | On-device `guard` | In the audit decision record only | Risk-tier the session |
| Detection reports | a category: `DEBUGGER`, `HOOKING_FRAMEWORK` or `ROOT_OR_JAILBREAK` | On-device `guard`, reported during a session | In the audit decision record only | See detections that occur between attestations |
| Anti-replay nonce | random bytes | Server-generated | Short-TTL store; expires/purged | Prevent replay of a challenge |
| Trust decisions | timestamp, event type, licence id, device id, decision, assurance level, reasons | Derived | Append-only audit log | Security audit trail |

### Data not processed or stored

The exclusions below are design decisions, not omissions.

- **No secrets in logs.** The audit record is secret-free by construction: no nonce,
  no private or session key, and no licence token material, only identifiers
  and the verdict.
- **No app content, user data, or obfuscated strings** ever reach the backend.
- **No private keys.** Only *public* device keys are stored. Session signing
  private keys are loaded from your own configuration at startup, with rotation
  support, and are never written to the stores described above.
- **No device hardware IDs.** IMEI and serial numbers are not requested from the
  attestation.
- **No detail of what a detection matched.** A detection report carries a
  category and nothing else — never the file, path or library that triggered
  it. Recording that would place the detector's own reference data into a log,
  and the log is readable on a device an attacker controls.

## Pseudonymity and PII

The only field that could identify an individual is `deviceId`, and it is
**client-supplied and opaque**: the integrator chooses what to put there, and a
random install UUID is the recommended choice. It is treated as **pseudonymous
data**, because it identifies a device but carries no name, email or contact
detail. It is used only to tell devices apart in the device list and the
decision log, and it is **not a security boundary**. An integrator who chooses to place actual PII in
`deviceId` makes that their own processing decision, which their DPA with their
end-users must cover.

## Retention

| Data | Retention |
| --- | --- |
| Nonces | Transient; expire at TTL, and loss on restart is acceptable |
| Attestation chains | Not retained; discarded after verification |
| License catalog / device keys | Lifetime of the license; deleted on license termination |
| Audit log | Per the deployment's retention policy; a bounded window (for example 90 to 365 days) with rotation is recommended |

The catalogue, device keys and revocations are held in the
service's data directory and survive a restart, so retention is a property of
that directory and of your backups of it.

## Security controls on the data

- Transport: terminate **TLS** in front of the service. It expects to sit
  behind your own ingress.
- Request binding: sensitive endpoints require a signed envelope authenticated
  against the device's attested key.
- Rate limiting per caller IP, with health and readiness probes exempt.

## Data-subject and deletion requests

Records are keyed by `(licenseId, deviceId)`, so a deletion request maps to a
specific device under a specific licence. The operator API resolves one
directly: removing the device deletes its device-key records. Audit entries are retained or removed under your own retention policy,
subject to whatever audit-retention law applies to you.

## Subprocessors

You self-host the service, so the data described here stays in your own
infrastructure and **you have no StringGuard subprocessors**. Any subprocessor
in scope is one you engage yourself, such as your cloud host or your managed
datastore.

This document describes the system's data behaviour as implemented, and is
intended as input to a Data Processing Agreement rather than a substitute for
one.
