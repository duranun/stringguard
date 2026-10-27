# Security policy

StringGuard is a security product, and its own code is held to the standard the
product asks of the applications that embed it. This document is the single
source of truth for how to report a vulnerability in StringGuard and what
happens after a report arrives.

## Reporting a vulnerability

Do not open a public issue for a security problem. Use GitHub's private
vulnerability reporting instead.

1. Open the repository's **Security** tab and choose **Report a vulnerability**
   (GitHub Security Advisories). This opens a private channel visible only to
   the maintainers and to you.
2. Include the affected version or commit, a description of the issue,
   reproduction steps or a proof-of-concept, and the impact you believe it has.

If you cannot use GitHub advisories, request a security contact through the same
Security tab and an encrypted channel will be provided. Do not post details in
public issues, discussions, or social media until a fix is released.

## Response targets

| Stage | Target |
| --- | --- |
| Acknowledgement of your report | within **3 business days** |
| Initial assessment (triaged + severity) | within **10 business days** |
| Fix or mitigation plan communicated | within **30 days** for High/Critical |
| Coordinated public disclosure | after a fix ships, by mutual agreement |

StringGuard follows coordinated disclosure. Reporters are credited in the
advisory and in the release notes unless they ask to remain anonymous.

## Scope

In scope:

- The Gradle plugin, the compiler plugin, the runtime and the annotations.
- The runtime self-protection (RASP) layer and tamper binding.
- The attestation client, the attestation verification service, and licence
  verification.
- The Swift integration and the Flutter packages.

Out of scope by design (see the security model section in the README):

- Recovery of obfuscated strings through **on-device dynamic analysis**: a
  debugger, Frida, runtime hooks, or a rooted device under the attacker's
  control. StringGuard raises the cost of *static* analysis. It is not a vault
  for a secret that must never leak, and a finding that a value can be read by
  attaching a debugger to a device you control is expected behaviour, not a bug.
- Issues that require a privileged local attacker (root or MDM) already past the
  RASP layer, unless they reveal a flaw in a control StringGuard claims to
  provide.
- Findings in third-party dependencies with no StringGuard-specific exploit
  path. Report those upstream; they are still welcome here.

## Supported versions

The **latest released minor line** receives security fixes. Customers under a
support agreement receive backports to an earlier line per that agreement.

