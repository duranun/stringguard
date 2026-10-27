import 'dart:typed_data';

import 'stringguard_flutter_platform_interface.dart';

export 'stringguard_flutter_platform_interface.dart' show GuardReport, AttestationResult;
export 'src/session.dart' show StringGuardClient, SessionResult, StringGuardHttpException;

/// StringGuard for Flutter: native RASP (guard) + hardware attestation client.
/// String obfuscation itself is build-time (the `stringguard` package + code
/// generator); this plugin surfaces the native device-trust layer.
class StringguardFlutter {
  /// Measures the current RASP signals natively.
  Future<GuardReport> guardReport({bool rootIsSuspicious = false}) =>
      StringguardFlutterPlatform.instance.guardReport(rootIsSuspicious: rootIsSuspicious);

  /// Produces (and persists) a hardware-attested key for a server-issued [nonce],
  /// keyed per [tenantId] so independent integrations never clobber each other.
  /// Returns the attestation cert chain to POST to the backend's /attest/verify.
  Future<AttestationResult> attest({required Uint8List nonce, required String tenantId}) =>
      StringguardFlutterPlatform.instance.attest(nonce: nonce, tenantId: tenantId);

  /// Signs a sensitive request with the attested key (request binding). Returns
  /// the ECDSA signature to attach as the envelope's `signature`, or null when
  /// signing is unavailable (no key / unsupported platform).
  Future<Uint8List?> signRequest({
    required String tenantId,
    required String sessionToken,
    required String method,
    required String path,
    required Uint8List bodySha256,
    required int timestampMs,
    required Uint8List requestNonce,
  }) =>
      StringguardFlutterPlatform.instance.signRequest(
        tenantId: tenantId,
        sessionToken: sessionToken,
        method: method,
        path: path,
        bodySha256: bodySha256,
        timestampMs: timestampMs,
        requestNonce: requestNonce,
      );

  /// Signs a proof of possession over a server nonce for [audience] (the tenant),
  /// for attest-once / prove-per-tenant. Null when signing is unavailable.
  Future<Uint8List?> signDeviceProof({
    required String tenantId,
    required String audience,
    required Uint8List nonce,
  }) =>
      StringguardFlutterPlatform.instance.signDeviceProof(tenantId: tenantId, audience: audience, nonce: nonce);

  /// Removes the attested key for [tenantId] from the platform keystore.
  Future<void> deleteDeviceKey({required String tenantId}) =>
      StringguardFlutterPlatform.instance.deleteDeviceKey(tenantId: tenantId);
}

