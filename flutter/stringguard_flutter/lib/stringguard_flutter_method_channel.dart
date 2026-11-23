
import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';

import 'stringguard_flutter_platform_interface.dart';

/// Method-channel implementation of [StringguardFlutterPlatform].
class MethodChannelStringguardFlutter extends StringguardFlutterPlatform {
  @visibleForTesting
  final methodChannel = const MethodChannel('stringguard_flutter');

  @override
  Future<GuardReport> guardReport({bool rootIsSuspicious = false}) async {
    final map = await methodChannel.invokeMapMethod<Object?, Object?>(
      'guardReport',
      {'rootIsSuspicious': rootIsSuspicious},
    );
    return GuardReport.fromMap(map ?? const {});
  }

  @override
  Future<AttestationResult> attest({required Uint8List nonce, required String tenantId}) async {
    final map = await methodChannel.invokeMapMethod<Object?, Object?>(
      'attest',
      {'nonce': nonce, 'tenantId': tenantId},
    );
    return AttestationResult.fromMap(map ?? const {'supported': false, 'reason': 'no response'});
  }

  @override
  Future<Uint8List?> signRequest({
    required String tenantId,
    required String sessionToken,
    required String method,
    required String path,
    required Uint8List bodySha256,
    required int timestampMs,
    required Uint8List requestNonce,
  }) =>
      methodChannel.invokeMethod<Uint8List>('signRequest', {
        'tenantId': tenantId,
        'sessionToken': sessionToken,
        'method': method,
        'path': path,
        'bodySha256': bodySha256,
        'timestampMs': timestampMs,
        'requestNonce': requestNonce,
      });

  @override
  Future<Uint8List?> signDeviceProof({
    required String tenantId,
    required String audience,
    required Uint8List nonce,
  }) =>
      methodChannel.invokeMethod<Uint8List>('signDeviceProof', {
        'tenantId': tenantId,
        'audience': audience,
        'nonce': nonce,
      });

  @override
  Future<List<Uint8List>?> deviceChain({required String tenantId}) async {
    final chain = await methodChannel.invokeListMethod<Object?>('deviceChain', {'tenantId': tenantId});
    return chain?.map((e) => Uint8List.fromList((e as List).cast<int>())).toList();
  }

  @override
  Future<void> deleteDeviceKey({required String tenantId}) =>
      methodChannel.invokeMethod<void>('deleteDeviceKey', {'tenantId': tenantId});
}
