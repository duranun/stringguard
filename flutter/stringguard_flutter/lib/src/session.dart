import 'dart:convert';
import 'dart:math';

import 'package:crypto/crypto.dart';
import 'package:flutter/foundation.dart';
import 'package:http/http.dart' as http;

import '../stringguard_flutter_platform_interface.dart';

/// The outcome of [StringGuardClient.establishSession].
class SessionResult {
  final String decision;
  final String? sessionToken;
  final String? tier;
  final List<String> reasons;

  const SessionResult({
    required this.decision,
    this.sessionToken,
    this.tier,
    this.reasons = const [],
  });

  /// True when the backend issued a usable session (not a DENY).
  bool get allowed => sessionToken != null && decision != 'DENY';

  @override
  String toString() => 'SessionResult(decision=$decision, tier=$tier, '
      'hasToken=${sessionToken != null}, reasons=$reasons)';
}

/// Thrown when an `/attest/*` call returns a non-2xx response.
class StringGuardHttpException implements Exception {
  final String path;
  final int status;
  final String body;
  StringGuardHttpException(this.path, this.status, this.body);
  @override
  String toString() => 'StringGuard HTTP $status on $path: $body';
}

/// One-call client for the StringGuard attestation protocol: it runs
/// challenge → attest → verify to get a session token, and builds the signed
/// envelope for each sensitive request — producing exactly the wire shapes the
/// backend expects, so integrators don't hand-roll base64/sha256/nonce glue and
/// can't drift from the protocol. Deliberately small: no retries, caching, or
/// prove-per-tenant flow here — those stay opt-in at the primitive level.
class StringGuardClient {
  StringGuardClient({
    required this.baseUrl,
    required this.licenseToken,
    required this.tenantId,
    http.Client? httpClient,
    StringguardFlutterPlatform? platform,
  })  : _http = httpClient ?? http.Client(),
        _platform = platform ?? StringguardFlutterPlatform.instance;

  final String baseUrl;
  final String licenseToken;
  final String tenantId;
  final http.Client _http;
  final StringguardFlutterPlatform _platform;
  final _rng = Random.secure();

  /// challenge → native attest + guardReport → verify. Returns the backend's
  /// decision and (on success) a short-lived session token.
  ///
  /// [platform] defaults to the platform the app is running on. It used to
  /// default to `android` and the App Attest result was never sent, so an iOS
  /// app using this client could only ever get a limited session.
  Future<SessionResult> establishSession({
    required String deviceId,
    String? platform,
  }) async {
    platform ??= defaultTargetPlatform == TargetPlatform.iOS ? 'ios' : 'android';
    final ch = await _postJson('/attest/challenge', {
      'licenseToken': licenseToken,
      'platform': platform,
    });
    final nonceB64 = ch['nonce'] as String;

    final att = await _platform.attest(nonce: base64Decode(nonceB64), tenantId: tenantId);
    final g = await _platform.guardReport();

    final verify = await _postJson('/attest/verify', {
      'licenseToken': licenseToken,
      'platform': platform,
      'nonce': nonceB64,
      'attestationChain': att.derChain.map(base64Encode).toList(),
      if (att.isAppAttest)
        'appleAttestation': {
          'keyId': att.keyId,
          'attestationObject': base64Encode(att.attestationObject!),
        },
      'integrity': {
        'packageName': '',
        'deviceId': deviceId,
        'debuggerAttached': g.debuggerAttached,
        'rootDetected': g.rootDetected,
        'hookingDetected': g.hookingDetected,
        'ts': DateTime.now().millisecondsSinceEpoch,
      },
    });
    return SessionResult(
      decision: verify['decision'] as String? ?? 'DENY',
      sessionToken: verify['sessionToken'] as String?,
      tier: verify['tier'] as String?,
      reasons: ((verify['reasons'] as List?) ?? const []).cast<String>(),
    );
  }

  /// Builds the signed request envelope (ready to POST) binding this request to
  /// the attested key. Hashes [body], uses a fresh nonce + timestamp, and signs
  /// via the native key. Returns null when the device cannot sign (no attested
  /// key / unsupported platform) — the caller decides whether to proceed.
  Future<Map<String, Object?>?> signedRequest({
    required String sessionToken,
    required String method,
    required String path,
    required Uint8List body,
  }) async {
    final bodySha = Uint8List.fromList(sha256.convert(body).bytes);
    final ts = DateTime.now().millisecondsSinceEpoch;
    final nonce = _nonce(16);
    final sig = await _platform.signRequest(
      tenantId: tenantId,
      sessionToken: sessionToken,
      method: method,
      path: path,
      bodySha256: bodySha,
      timestampMs: ts,
      requestNonce: nonce,
    );
    if (sig == null) return null;
    return {
      'sessionToken': sessionToken,
      'method': method,
      'path': path,
      'bodySha256': base64Encode(bodySha),
      'timestamp': ts,
      'requestNonce': base64Encode(nonce),
      'signature': base64Encode(sig),
    };
  }

  /// Releases the underlying HTTP client (if this client created it).
  void close() => _http.close();

  Uint8List _nonce(int n) => Uint8List.fromList(List<int>.generate(n, (_) => _rng.nextInt(256)));

  Future<Map<String, Object?>> _postJson(String path, Map<String, Object?> body) async {
    final resp = await _http.post(
      Uri.parse('$baseUrl$path'),
      headers: const {'content-type': 'application/json'},
      body: jsonEncode(body),
    );
    if (resp.statusCode ~/ 100 != 2) {
      throw StringGuardHttpException(path, resp.statusCode, resp.body);
    }
    return jsonDecode(resp.body) as Map<String, Object?>;
  }
}
