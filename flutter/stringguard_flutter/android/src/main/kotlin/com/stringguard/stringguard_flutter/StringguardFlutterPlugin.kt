package com.stringguard.stringguard_flutter

import com.stringguard.attest.AttestationOutcome
import com.stringguard.attest.deviceKeyAlias
import com.stringguard.attest.platformAttestPersistent
import com.stringguard.attest.platformDeleteKey
import com.stringguard.attest.signDeviceProof
import com.stringguard.attest.signRequest
import com.stringguard.guard.Guard
import io.flutter.embedding.engine.plugins.FlutterPlugin
import io.flutter.plugin.common.MethodCall
import io.flutter.plugin.common.MethodChannel
import io.flutter.plugin.common.MethodChannel.MethodCallHandler
import io.flutter.plugin.common.MethodChannel.Result

/** StringguardFlutterPlugin */
class StringguardFlutterPlugin :
    FlutterPlugin,
    MethodCallHandler {
    // The MethodChannel that will the communication between Flutter and native Android
    //
    // This local reference serves to register the plugin with the Flutter Engine and unregister it
    // when the Flutter Engine is detached from the Activity
    private lateinit var channel: MethodChannel

    override fun onAttachedToEngine(flutterPluginBinding: FlutterPlugin.FlutterPluginBinding) {
        channel = MethodChannel(flutterPluginBinding.binaryMessenger, "stringguard_flutter")
        channel.setMethodCallHandler(this)
    }

    override fun onMethodCall(
        call: MethodCall,
        result: Result
    ) {
        when (call.method) {
            "guardReport" -> {
                // Set the root-is-suspicious policy if the caller passed one.
                (call.argument<Boolean>("rootIsSuspicious"))?.let { Guard.rootIsSuspicious = it }
                val r = Guard.report()
                result.success(
                    mapOf(
                        "debuggerAttached" to r.debuggerAttached,
                        "rootDetected" to r.rootDetected,
                        "hookingDetected" to r.hookingDetected,
                        "suspicious" to r.suspicious,
                    ),
                )
            }
            "attest" -> {
                val nonce = call.argument<ByteArray>("nonce")!!
                val alias = deviceKeyAlias(call.argument<String>("tenantId")!!)
                when (val o = platformAttestPersistent(nonce, alias)) {
                    is AttestationOutcome.CertChain ->
                        result.success(mapOf("supported" to true, "derChain" to o.derChain, "strongBox" to o.strongBox))
                    is AttestationOutcome.Unsupported ->
                        result.success(mapOf("supported" to false, "reason" to o.reason))
                    // App Attest is iOS-only; Android never produces it.
                    is AttestationOutcome.AppAttest ->
                        result.success(mapOf("supported" to false, "reason" to "unexpected App Attest outcome on Android"))
                }
            }
            "signRequest" -> {
                val env = signRequest(
                    deviceKeyAlias(call.argument<String>("tenantId")!!),
                    call.argument<String>("sessionToken")!!,
                    call.argument<String>("method")!!,
                    call.argument<String>("path")!!,
                    call.argument<ByteArray>("bodySha256")!!,
                    (call.argument<Number>("timestampMs")!!).toLong(),
                    call.argument<ByteArray>("requestNonce")!!,
                )
                result.success(env?.signature)
            }
            "signDeviceProof" -> {
                val sig = signDeviceProof(
                    deviceKeyAlias(call.argument<String>("tenantId")!!),
                    call.argument<String>("audience")!!,
                    call.argument<ByteArray>("nonce")!!,
                )
                result.success(sig)
            }
            "deleteDeviceKey" -> {
                platformDeleteKey(deviceKeyAlias(call.argument<String>("tenantId")!!))
                result.success(null)
            }
            else -> result.notImplemented()
        }
    }

    override fun onDetachedFromEngine(binding: FlutterPlugin.FlutterPluginBinding) {
        channel.setMethodCallHandler(null)
    }
}
