import 'dart:typed_data';

import 'package:plugin_platform_interface/plugin_platform_interface.dart';

import 'stringguard_flutter_method_channel.dart';

/// A snapshot of the native RASP signals (mirrors the Kotlin `GuardReport`).
class GuardReport {
  final bool debuggerAttached;
  final bool rootDetected;
  final bool hookingDetected;
  final bool suspicious;

  const GuardReport({
    required this.debuggerAttached,
    required this.rootDetected,
    required this.hookingDetected,
    required this.suspicious,
  });

  factory GuardReport.fromMap(Map<Object?, Object?> m) => GuardReport(
        debuggerAttached: m['debuggerAttached'] == true,
        rootDetected: m['rootDetected'] == true,
        hookingDetected: m['hookingDetected'] == true,
        suspicious: m['suspicious'] == true,
      );

  @override
  String toString() =>
      'GuardReport(debugger=$debuggerAttached, root=$rootDetected, '
      'hooking=$hookingDetected, suspicious=$suspicious)';
}

/// Outcome of [StringguardFlutterPlatform.attest].
///
/// The two platforms produce different evidence, and it is not forced into one
/// shape. Android returns a certificate chain in [derChain]; Apple App Attest
/// returns a [keyId] and a CBOR [attestationObject], which is not a chain. One
/// of the two is populated and the other is empty, so a caller should branch on
/// which is present rather than assume. The server's verify request makes the
/// same split.
///
/// When [supported] is false, [reason] explains why and the caller degrades to
/// the software tier. On Android that is a pre-API-26 device or an emulator
/// without a trusted execution environment; on Apple it is the Simulator, a
/// device without a Secure Enclave, or a missing App Attest capability on the
/// provisioning profile.
class AttestationResult {
  final bool supported;
  final String? reason;

  /// Android: the leaf-first attestation certificate chain.
  final List<Uint8List> derChain;

  /// Android: whether the key was requested in StrongBox. A client hint only;
  /// the server reads the true security level from the chain.
  final bool strongBox;

  /// Apple: the identifier of the attested Secure Enclave key. Needed again to
  /// produce assertions, and sent to the server, which re-derives it from the
  /// attested key rather than trusting it.
  final String? keyId;

  /// Apple: the CBOR attestation object from `DCAppAttestService.attestKey`.
  final Uint8List? attestationObject;

  const AttestationResult({
    required this.supported,
    this.reason,
    this.derChain = const [],
    this.strongBox = false,
    this.keyId,
    this.attestationObject,
  });

  /// True when this carries Apple App Attest evidence rather than a chain.
  bool get isAppAttest => keyId != null && attestationObject != null;

  factory AttestationResult.fromMap(Map<Object?, Object?> m) => AttestationResult(
        supported: m['supported'] == true,
        reason: m['reason'] as String?,
        derChain: ((m['derChain'] as List?) ?? const [])
            .map((e) => Uint8List.fromList((e as List).cast<int>()))
            .toList(),
        strongBox: m['strongBox'] == true,
        keyId: m['keyId'] as String?,
        attestationObject: m['attestationObject'] == null
            ? null
            : Uint8List.fromList((m['attestationObject'] as List).cast<int>()),
      );

  @override
  String toString() => isAppAttest
      ? 'AttestationResult(supported=$supported, appAttest, '
          'object=${attestationObject!.length} bytes'
          '${reason != null ? ', reason=$reason' : ''})'
      : 'AttestationResult(supported=$supported, certs=${derChain.length}, '
          'strongBox=$strongBox${reason != null ? ', reason=$reason' : ''})';
}

abstract class StringguardFlutterPlatform extends PlatformInterface {
  StringguardFlutterPlatform() : super(token: _token);

  static final Object _token = Object();
  static StringguardFlutterPlatform _instance = MethodChannelStringguardFlutter();
  static StringguardFlutterPlatform get instance => _instance;
  static set instance(StringguardFlutterPlatform instance) {
    PlatformInterface.verifyToken(instance, _token);
    _instance = instance;
  }

  Future<GuardReport> guardReport({bool rootIsSuspicious = false}) =>
      throw UnimplementedError('guardReport() has not been implemented.');

  Future<AttestationResult> attest({required Uint8List nonce, required String tenantId}) =>
      throw UnimplementedError('attest() has not been implemented.');

  Future<Uint8List?> signRequest({
    required String tenantId,
    required String sessionToken,
    required String method,
    required String path,
    required Uint8List bodySha256,
    required int timestampMs,
    required Uint8List requestNonce,
  }) =>
      throw UnimplementedError('signRequest() has not been implemented.');

  Future<Uint8List?> signDeviceProof({
    required String tenantId,
    required String audience,
    required Uint8List nonce,
  }) =>
      throw UnimplementedError('signDeviceProof() has not been implemented.');

  /// Android: the certificate chain of the key attested earlier for [tenantId],
  /// leaf first, or null when there is none. Null on iOS.
  Future<List<Uint8List>?> deviceChain({required String tenantId}) =>
      throw UnimplementedError('deviceChain() has not been implemented.');

  Future<void> deleteDeviceKey({required String tenantId}) =>
      throw UnimplementedError('deleteDeviceKey() has not been implemented.');
}
