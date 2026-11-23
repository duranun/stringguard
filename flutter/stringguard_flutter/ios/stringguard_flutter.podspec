#
# To learn more about a Podspec see http://guides.cocoapods.org/syntax/podspec.html.
# Run `pod lib lint stringguard_flutter.podspec` to validate before publishing.
#
Pod::Spec.new do |s|
  s.name             = 'stringguard_flutter'
  s.version          = '1.1.0'
  s.summary          = 'StringGuard runtime self-protection and device attestation for Flutter.'
  s.description      = <<-DESC
Flutter plugin exposing StringGuard's runtime self-protection (RASP) and device
attestation to Dart. Proprietary; see LICENSE.
                       DESC
  s.homepage         = 'https://github.com/duranun/stringguard'
  s.license          = { :file => '../LICENSE' }
  s.author           = { 'Duran Un' => 'duranun@gmail.com' }
  s.source           = { :path => '.' }
  s.source_files = 'stringguard_flutter/Sources/stringguard_flutter/**/*'
  # Kotlin/Native binary holding guard and attest-core. Produced by
  # ../build-ios-framework.sh, which must be run before pod install.
  s.vendored_frameworks = 'stringguard_flutter/StringGuard.xcframework'
  s.dependency 'Flutter'
  s.platform = :ios, '15.0'

  # Flutter.framework does not contain a i386 slice.
  s.pod_target_xcconfig = { 'DEFINES_MODULE' => 'YES', 'EXCLUDED_ARCHS[sdk=iphonesimulator*]' => 'i386' }
  s.swift_version = '5.0'

  # Required, not optional: the plugin reads and writes UserDefaults, which is a
  # required-reason API. Without this bundle an app embedding the plugin is
  # warned on upload (ITMS-91053), and the declaration has to come from the SDK.
  s.resource_bundles = {'stringguard_flutter_privacy' => ['stringguard_flutter/Sources/stringguard_flutter/PrivacyInfo.xcprivacy']}
end
