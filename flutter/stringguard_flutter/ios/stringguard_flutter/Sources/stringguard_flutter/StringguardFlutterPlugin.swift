import Flutter
import StringGuard
import UIKit

/// iOS side of the StringGuard method channel.
///
/// This used to be a fail-open stub: every method answered `false` or
/// `unsupported`, so a Flutter application on iOS believed the runtime
/// self-protection layer was present and silent when in fact nothing was
/// running. It now calls the real Kotlin/Native implementation through
/// `StringGuard.xcframework`.
///
/// Two of the five methods still cannot work on this platform, and they say so
/// rather than answering `false`. The difference matters: a caller can act on
/// "not available on iOS", and cannot act on a silent negative.
public class StringguardFlutterPlugin: NSObject, FlutterPlugin {
  /// Where the Apple App Attest key id is remembered between `attest` and
  /// `signDeviceProof`.
  ///
  /// Android derives a KeyStore alias from the tenant id and the key persists
  /// there. Apple instead hands back a key id when the key is created, and that
  /// id is the only way to use the key again, so it has to be stored.
  ///
  /// `UserDefaults` rather than the keychain, deliberately: the key id is a
  /// public identifier, the SHA-256 of the attested public key, and it is sent
  /// to the server in the clear. The Secure Enclave key it names is destroyed
  /// when the application is uninstalled, so there is no secret here to protect
  /// and nothing that outlives the install.
  private static let keyIdPrefix = "com.stringguard.appattest.keyId."

  public static func register(with registrar: FlutterPluginRegistrar) {
    let channel = FlutterMethodChannel(
      name: "stringguard_flutter", binaryMessenger: registrar.messenger())
    let instance = StringguardFlutterPlugin()
    registrar.addMethodCallDelegate(instance, channel: channel)
  }

  private func args(_ call: FlutterMethodCall) -> [String: Any] {
    call.arguments as? [String: Any] ?? [:]
  }

  private func data(_ value: Any?) -> Data? {
    if let typed = value as? FlutterStandardTypedData { return typed.data }
    if let bytes = value as? [UInt8] { return Data(bytes) }
    return nil
  }

  private func storedKeyId(_ tenantId: String) -> String? {
    UserDefaults.standard.string(forKey: Self.keyIdPrefix + tenantId)
  }

  private func storeKeyId(_ keyId: String, for tenantId: String) {
    UserDefaults.standard.set(keyId, forKey: Self.keyIdPrefix + tenantId)
  }

  public func handle(_ call: FlutterMethodCall, result: @escaping FlutterResult) {
    switch call.method {
    case "guardReport":
      handleGuardReport(call, result)
    case "attest":
      handleAttest(call, result)
    case "signDeviceProof":
      handleSignDeviceProof(call, result)

    case "signRequest":
      // Request binding signs an arbitrary canonical payload with the attested
      // key. App Attest does not expose the key for that: it only produces
      // assertions, whose shape the server's request authenticator does not
      // accept. `platformSign` in attest-core returns nil on Apple targets for
      // the same reason, and the server has no Apple path either. Returning nil
      // is the documented degrade; the caller falls back to an unbound request.
      result(nil)

    case "deviceChain":
      // Android only. An iOS refresh proves the key with an assertion instead
      // of resending a chain, because App Attest has none to resend.
      result(nil)

    case "deleteDeviceKey":
      // Apple provides no API to delete an App Attest key. The key lives in the
      // Secure Enclave for the life of the installation and goes with it. The
      // remembered key id is dropped, so the next `attest` generates a new key
      // and the old one is simply never used again.
      if let tenantId = args(call)["tenantId"] as? String {
        UserDefaults.standard.removeObject(forKey: Self.keyIdPrefix + tenantId)
      }
      result(nil)

    default:
      result(FlutterMethodNotImplemented)
    }
  }

  // MARK: - guardReport

  private func handleGuardReport(_ call: FlutterMethodCall, _ result: @escaping FlutterResult) {
    if let rootIsSuspicious = args(call)["rootIsSuspicious"] as? Bool {
      Guard.shared.rootIsSuspicious = rootIsSuspicious
    }
    let report = Guard.shared.report()
    result([
      "debuggerAttached": report.debuggerAttached,
      "rootDetected": report.rootDetected,
      "hookingDetected": report.hookingDetected,
      "suspicious": report.suspicious,
    ])
  }

  // MARK: - attest

  private func handleAttest(_ call: FlutterMethodCall, _ result: @escaping FlutterResult) {
    let a = args(call)
    guard let nonce = data(a["nonce"]), let tenantId = a["tenantId"] as? String else {
      result(FlutterError(code: "bad_args", message: "attest needs nonce and tenantId", details: nil))
      return
    }

    Task {
      let statement: AttestationStatement
      do {
        statement = try await StringGuardKit.shared.attest(nonce: nonce)
      } catch {
        // Never throws into Dart: the client does not decide whether it is
        // trustworthy, it reports what it could produce.
        result(["supported": false, "reason": "App Attest failed: \(error.localizedDescription)"])
        return
      }

      let outcome = statement.outcome
      if let unsupported = outcome as? AttestationOutcomeUnsupported {
        // False on the Simulator, on devices without a Secure Enclave, and
        // when the App Attest capability is missing from the provisioning
        // profile.
        result(["supported": false, "reason": unsupported.reason])
        return
      }
      guard let keyId = StringGuardKit.shared.keyId(outcome: outcome),
            let attestationObject = StringGuardKit.shared.attestationObject(outcome: outcome)
      else {
        result(["supported": false, "reason": "unexpected attestation outcome"])
        return
      }

      // Remembered so signDeviceProof can use the same key.
      self.storeKeyId(keyId, for: tenantId)

      // Apple App Attest is not a certificate chain, so it travels in its own
      // shape rather than being squeezed into derChain. This mirrors the
      // appleAttestation field on the server's verify request.
      result([
        "supported": true,
        "keyId": keyId,
        "attestationObject": FlutterStandardTypedData(bytes: attestationObject),
      ])
    }
  }

  // MARK: - signDeviceProof

  private func handleSignDeviceProof(_ call: FlutterMethodCall, _ result: @escaping FlutterResult) {
    let a = args(call)
    guard let audience = a["audience"] as? String,
          let nonce = data(a["nonce"]),
          let tenantId = a["tenantId"] as? String
    else {
      result(FlutterError(
        code: "bad_args", message: "signDeviceProof needs tenantId, audience and nonce",
        details: nil))
      return
    }

    // Apple attestation is one-shot per key, so the key must already exist.
    // Without a stored key id there is nothing to prove with, and the caller
    // has to call attest first.
    guard let keyId = storedKeyId(tenantId) else {
      result(nil)
      return
    }

    Task {
      do {
        let assertion = try await StringGuardKit.shared.assertion(
          keyId: keyId, audience: audience, nonce: nonce)
        // nil is the documented degrade path, not an error.
        result(assertion.map { FlutterStandardTypedData(bytes: $0) })
      } catch {
        result(nil)
      }
    }
  }
}
