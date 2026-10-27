# StringGuard Attestation

Attestation is an optional layer that answers a question obfuscation and
on-device checks cannot: **is this install a genuine one, of your app, on a
sound device?**

It is stronger than client-side runtime protection because the decision is made
on a server, from signed material the app cannot forge. A client-side check can
be hooked once and stays hooked; a server-side decision has to be defeated live,
per session, against a service that can see it happening and revoke.

This document is for integrators wiring the layer into an application, and for
reviewers assessing what it does and does not prove. It is self-hosted: the
verification service runs in your own infrastructure, so device traffic never
leaves it. Section 7 lists the operational responsibilities that come with
that.

---

## 1. What it proves

| Assurance | Android | iOS | Can the service trust "genuine"? |
|---|---|---|---|
| **Hardware** | hardware-backed key attestation (TEE or StrongBox) | Apple App Attest | **Yes.** The statement is rooted in hardware the device cannot forge in software. |
| **Software** | signed statement, app-held key | signed statement, Secure Enclave key without Apple attestation | **No.** Forgeable by an attacker who extracts the key — but centralized and revocable. |

At the hardware assurance level the service also learns the device's verified
boot state and bootloader lock state, and the package name and signing
certificate of the app that produced the statement. So it can tell your app
apart from a repackaged copy of it.

Verification is offline against pinned platform roots. There is no dependency on
Play Integrity, no Google API call, and no request quota.

### What it does not prove

State these plainly in your risk acceptance:

- **Without a hardware root, the statement is only as strong as a key inside
  your app binary**, which is extractable. That is runtime-protection-strength
  assurance, centralized and revocable, not proof of a genuine device.
- **A compromised secure element or boot chain defeats even the hardware
  level.** The bar is high; it is not absolute.
- **Attestation says the device was sound when it attested**, not that it is
  sound right now. Layer your own key-age policy on top; each session's
  freshness proof covers possession, not continued integrity.
- **It does not protect the values StringGuard obfuscates.** Attestation and
  obfuscation answer different questions. An attacker with a debugger on their
  own device still recovers protected strings; see
  [SECURITY.md](../SECURITY.md).

---

## 2. Platform requirements

| | Requirement |
|---|---|
| Android | API 26+ for hardware-backed attestation (API 28+ for StrongBox). Older devices fall back to the software assurance level. |
| iOS | Apple App Attest, which needs the **App Attest capability** on the app's provisioning profile and a **physical device**. The Simulator and devices without a Secure Enclave report "unsupported" rather than failing. |
| Other targets | Not supported. Calls report "unsupported" rather than erroring. |

Unsupported is a normal branch, not an error: every client call returns a result
you can inspect and none of them throws.

---

## 3. How the decision reaches you

```
Your app ──► attestation service ──► session token
                (decision + assurance level + the device's attested key,
                 signed by the service)

Your app ──► your own API, presenting the session token
Your API ──► verifies the token's signature with the service's published
             public key: offline, one call, no round trip back
```

The session token is self-contained and short-lived. Because it carries the
device's attested public key inside the signed payload, your backend can also
verify that a request was signed by **that device** entirely offline — the
session key authenticates the device key, and the device key authenticates the
request. You need no shared device-key store and no webhook for the per-request
check.

A webhook is for asynchronous events only, such as a revocation push, never for
the per-request decision.

---

## 4. Integrating the client

Add the client library to your shared module:

```kotlin
// commonMain
implementation("com.stringguard:attest-core:<version>")
```

On iOS, read
[INTEGRATION.md §3.4](INTEGRATION.md#34-ios-linking-the-framework) first if your
app already links its own Kotlin framework — it cannot link two.

The client never decides its own trust. It produces the best attestation it can
for a nonce your service issued, carries the on-device runtime signals alongside
it, and signs sensitive requests with the attested key.

```kotlin
// 1. Ask your service for a nonce, then attest. Keep the key so it can sign
//    later requests. On Apple, use the suspending form: App Attest is
//    asynchronous and there is no synchronous path.
val alias = deviceKeyAlias(tenantId)          // see §6 if you ship an SDK
val outcome = platformAttestPersistent(nonce, alias)

// 2. Fold in the runtime signals. The attestation client does not depend on the
//    RASP library, so your app maps the fields across.
val r = Guard.report()
val signals = DeviceSignals(r.debuggerAttached, r.rootDetected, r.hookingDetected)
val statement = buildAttestationStatement(nonce, signals, System.currentTimeMillis())

// 3. Send the statement to your service; it returns a session token.

// 4. Bind each sensitive request to the attested key.
val envelope = signRequest(
    alias, sessionToken, "POST", "/secret", bodySha256,
    System.currentTimeMillis(), freshRandomNonce,
)
```

Request binding (step 4) is available on Android. If you need it on iOS, raise
it before you design around it.

---

## 5. Policy: start fail-open

A hard deny on failed hardware attestation will lock out paying customers. The
cohorts that legitimately fail are small but real, and they are not attackers:
older devices, rooted or bootloader-unlocked devices including your own
developers', hardened ROMs whose verified boot uses their own key, and
manufacturers with incomplete attestation provisioning.

The shipped defaults follow from that, and we recommend keeping them:

- **Fail-open.** Attestation *labels* a session's assurance level. The default
  is allow-and-report, never a blanket block.
- **Graduated, not binary.** High assurance gets full access; low or unknown
  assurance is allowed, with at most a step-up or a reduced limit on one
  specific action. A hard gate is opt-in and per-operation — key release, a
  payment — never app-wide login or basic use.
- **Monitor first.** Run in observe-only mode, collect the assurance
  distribution from real traffic, and tune policy before you enforce anything.
- **Policy is server-side and hot-swappable.** You can loosen it instantly
  during an outage or for a false-positive cohort without shipping an app
  update. A client-only check cannot do that.
- **You own the risk appetite.** We ship the signal, the assurance levels and
  safe defaults; what a low level costs is your decision.

A configuration or capability gap degrades rather than locks out: a client that
cannot attest at all gets a limited-assurance allow with an explicit reason, and
still receives a session token. A supplied attestation that **fails
verification** is an attack signal and is denied outright.

---

## 6. If you ship an SDK

StringGuard can end up in one app process twice: inside your SDK, and again in
your customer's own integration. Two things to know.

**Use a tenant-scoped key alias.** `deviceKeyAlias(tenantId)` derives a stable,
collision-free alias from your tenant identifier. Two integrations sharing one
alias would clobber each other's attested key. Pass your own tenant id; there is
no shared default to collide on.

**Attest once, prove per tenant.** The expensive hardware attestation runs once
to establish the device key. Any other tenant accepts that established device
together with a fresh, cheap proof-of-possession signature over its own nonce.
Two tenants then cost one attestation and two signatures instead of two full
attestations and two round trips at startup. The proof is bound to its audience,
so a proof minted for one tenant cannot be replayed to another.

**Keep your app talking to your own domain.** Hold your StringGuard licence
server-side and re-expose the endpoints through your own API. Your customers'
apps then make no network call to a third party they did not choose, and your
licence never ships inside an application.

Do not shade or rename the StringGuard packages. Shading breaks the key sharing
this section describes, so each integration pays for a full attestation of its
own; it also interferes with the runtime guard. If you have no alternative,
tenant-scoped aliases still prevent collisions, but expect the double cost.

---

## 7. Running the service

The service runs in your infrastructure, so device traffic and the records
below never leave it. It is delivered as an archive you unpack and run.

### 7.1 What you need

| | |
|---|---|
| Java | 17 or newer. The service tells you plainly if the runtime it finds is too old. |
| A data directory | Writable, on storage that survives a restart, and **included in your backups**. It holds your licence catalogue and the list of devices that have used your app. |
| TLS in front | The service expects to sit behind your own ingress and speaks plain HTTP to it. |
| Attestation roots | The Google hardware-attestation roots and the Apple App Attest root, as files you supply. Without at least one, the service refuses to start rather than silently providing no assurance. |
| Session signing keys | Supplied by you, from your own key custody. Your backend verifies session tokens against the matching public keys. |

### 7.2 Settings

All configuration is read from the environment.

| Setting | |
|---|---|
| `SG_PROFILE` | Set to `production`. Anything else runs permissively and reports that it is doing so. |
| `SG_LICENSE_PUBLIC_KEY` | The licence public key supplied with your licence. |
| `SG_SESSION_KEYS` | `kid:privateKey:publicKey`, base64. Append `;kid2::publicKey` to keep a retired key verifiable. |
| `SG_ANDROID_ROOTS` | Paths to the pinned Google hardware-attestation root certificates (DER), separated by `;`. |
| `SG_APPLE_ROOT` | Path to the pinned Apple App Attest root certificate (DER). |
| `SG_DATA_DIR` | The data directory. Defaults to `./data`. |
| `SG_HOST`, `SG_PORT` | Where the device API listens. Defaults to all interfaces on 8787. |
| `SG_ADMIN_HOST`, `SG_ADMIN_PORT` | Where the operator API listens. Loopback by default, and not listening at all until you set a port. |
| `SG_ADMIN_TOKEN` | The operator credential — used by both the API and the dashboard. At least 32 random characters. |
| `SG_DASHBOARD` | Set to `off` to serve the API without the dashboard. |
| `SG_AUDIT_LOG` | Path to the decision log. Omit it and no log is written. |
| `SG_SESSION_TTL_MS` | How long a session token stays valid. Defaults to 15 minutes. |
| `SG_RATE_PER_MIN` | Requests per minute per caller on the device API. Defaults to 120. |
| `SG_THREADS` | Request concurrency. Sized from the host by default. |
| `SG_ALLOW_APPLE_DEVELOPMENT` | Leave unset. Enabling it in production is refused. |

Only one platform's roots are needed if you ship for only one platform; the
other platform's clients then receive the limited tier.

### 7.3 Check it before you go live

```
bin/stringguard-attest doctor
```

This checks the configuration and starts nothing. It reports what is missing,
what is unsafe and what to do about each, and it exits non-zero if the
configuration is not fit to deploy. Run it as part of your install, and send us
its output if you need support — it is the fastest way for us to see a
deployment we cannot reach.

`run` performs the same check before it serves, and stops rather than starting
in a degraded state.

### 7.4 Run it

```
bin/stringguard-attest run
```

**Run exactly one instance.** This is a single-instance service, and the
constraint is not a formality: each instance holds your catalogue, device list
and in-flight challenges, so two of them disagree about all three. A licence
added to one is invisible to the other, each keeps its own device list, and a
device that gets its challenge from one instance and
presents it to the other is rejected — which looks like attestation failing at
random rather than like a deployment mistake.

A second instance started against the same data directory refuses to start and
says why, so the common case cannot happen by accident. Give each instance its
own directory and nothing detects it, so if you need redundancy, prefer a
single instance your orchestrator restarts over two serving at once.

Two endpoints are there for your orchestrator:

| | |
|---|---|
| `GET /healthz` | Whether the process is up. |
| `GET /readyz` | Whether it should be sent traffic. Returns 503 when it should not — for example when the data directory has become unwritable — and recovers on its own. |

Point your liveness probe at the first and your readiness probe at the second.
A restart on a failing readiness probe will not help; taking the instance out
of rotation will.

### 7.5 Day-to-day operation

A licence reaches the catalogue through the operator surface, and so does every
later intervention. There are two ways in — a dashboard and an API — and they
share one listener and one credential.

**It listens on its own port**, on loopback unless you say otherwise, and only
once you set `SG_ADMIN_PORT` and `SG_ADMIN_TOKEN`. Keep it off the path your
application reaches: it creates and revokes licences.

#### The dashboard

At `/ui/` on the operator port. Sign in with `SG_ADMIN_TOKEN`; there is no
separate account to create.

It has three pages:

- **Overview** — whether anything needs attention right now: devices that
  were blocked, marked suspicious or reported a threat in the last 24 hours,
  one line per device.
- **Licences** — each licence, the app identity devices must match, and the
  devices that have used it with when each was last seen. Add, update or revoke
  a licence here, or remove a device that was lost or replaced.
- **Security events** — what happened on each device check and why, problems
  only by default. Operator changes are listed separately underneath.

Results are in plain language. StringGuard checks each device and writes the
result into the session it issues; it restricts nothing itself. Your own server
reads that result and decides what the device may do, and only a blocked device
gets no session at all.

A licence is not limited to a number of devices. It grants the right to use
StringGuard; every device running your app is checked the same way.

Two things to know before you reach it:

- **It needs a secure context.** The session cookie is marked `Secure`, so
  signing in works over HTTPS or from `localhost` — an SSH tunnel to the
  loopback port is the simplest route — and not over plain HTTP to a remote
  address. If the sign-in form keeps returning, that is why.
- **The pages carry no JavaScript**, and are served under a content-security
  policy that permits none. Worth noting in a security review: there is no
  script for injected content to execute.

#### The API

For your own tooling — a provisioning step in your billing system, for example.
Every call carries `Authorization: Bearer $SG_ADMIN_TOKEN`.

| | |
|---|---|
| `PUT /admin/licences/{id}` | Add or update a licence: the app identity attestations must match. |
| `GET /admin/licences` | The catalogue, with how many devices have used each licence and whether it is revoked. |
| `POST /admin/licences/{id}/revoke` | Revoke. Takes effect on the next call from any device. |
| `GET /admin/licences/{id}/devices` | The devices that have used the licence. |
| `DELETE /admin/licences/{id}/devices/{deviceId}` | Remove a device and everything stored about it. This is also how you resolve a deletion request for a device. |
| `GET /admin/licences/{id}/devices/{deviceId}/keys` | The keys that device has attested with, for a rotation or incident review. |

Adding a licence looks like this:

```
curl -X PUT -H "Authorization: Bearer $SG_ADMIN_TOKEN" \
  -d '{"packageNames":["com.example.app"],
       "signingCertSha256":["<sha256 of your signing certificate>"],
       "appleAppId":"TEAMID.com.example.app"}' \
  "http://127.0.0.1:$SG_ADMIN_PORT/admin/licences/your-licence-id"
```

A licence naming no app on either platform is refused, on both surfaces: it
would accept an attestation from any application holding the licence token.

#### Either way

Changes that alter what the service allows — creating or updating a licence,
revoking one, removing a device — are written to the decision log, with
the surface they came through. Reads are not logged.

#### Seeing what happened

The **Security events** page shows the decision log newest first: the result
of every device check and the reasons behind it. A licence's own page shows its
recent problems and links to the same view narrowed to that licence. It needs
`SG_AUDIT_LOG` set; without it the page says so rather than showing an empty
table.

The runtime protection's signals are part of this. Every attestation carries
the device's debugger, hooking and root state alongside it, so a session that
was flagged or denied for one of them says so in its reasons — "client reported
root" and the like — and that is visible here per device and per licence.

For the gap in between — a debugger that attaches ten minutes into a session —
the client can report a detection as it happens:

```
POST /attest/report { "sessionToken": "...", "categories": ["HOOKING_FRAMEWORK"] }
```

It is authenticated by the session token the client already holds, so a report
is trusted exactly as far as the session it came from, and the licence and
device it is recorded against come from that signed token rather than from the
request. Reports appear in the decision log as `CLIENT_REPORT`.

**A report is not a control.** Nothing about it changes a decision, revokes
anything or denies a session. A signal from a device under an attacker's
control cannot be enforced on; across a fleet it is still worth seeing, which
is why it is recorded and shown rather than acted upon. One device reporting a
hooking framework means little; four hundred on one release is something to
act on.

A deployment that uses the obfuscation and runtime-protection layers without
this service has nowhere to send reports, so there the callback described in
the runtime guard section of the README is the whole story.

### 7.6 What remains yours

| | |
|---|---|
| TLS termination, and passing the real client address | So that rate limiting counts per caller rather than per proxy. |
| Custody of the session signing private keys | A KMS or HSM rather than the environment, if your policy requires it. |
| Keeping the pinned roots current | Platform roots change; a stale set rejects genuine devices. |
| Backups of the data directory | Losing it loses your catalogue and your device history. |
| Capacity | Size the one instance for your traffic — `SG_THREADS` and its memory limit, not more replicas. |
| Audit-log retention | Per your own policy. |

One habit worth adopting from the start: **rotate session keys by adding, not
replacing.** Add the new current key, keep the old one for verification, and
drop it once its tokens have expired. Replacing outright invalidates every
session token in flight.

## 8. Limits to record

- Hardware attestation is unavailable on some devices and on emulators. Policy
  must degrade to the software level rather than deny, or you will lock out real
  users.
- For what the service processes, stores and never touches — the input to your
  DPA — see [DATA_HANDLING.md](DATA_HANDLING.md).
