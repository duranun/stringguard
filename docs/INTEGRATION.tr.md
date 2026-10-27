# StringGuard Entegrasyon Kılavuzu

[English](INTEGRATION.md) · **Türkçe**

Bu kılavuz mevcut bir Kotlin projesine StringGuard'ı baştan sona entegre
etmenizi anlatır: toolchain gereksinimleri, kurulum, platform bazında modül
ayarları, ilk build, obfuscation kapsamı, CI, attestation ve sorun giderme. Entegrasyonu
yapan geliştirici için yazıldı; kısa versiyonu [README](../README.tr.md)'de.

**İçindekiler**
1. [Gereksinimler](#1-gereksinimler)
2. [Kurulum](#2-kurulum)
   - [Repository tanımı](#21-repository-tanımı)
3. [StringGuard'ı ekleme](#3-stringguardı-ekleme)
   - [Klasik Android](#31-klasik-android-kmp-olmayan)
   - [JVM / Desktop](#32-jvm--desktop)
   - [Kotlin Multiplatform / Compose Multiplatform](#33-kotlin-multiplatform--compose-multiplatform)
   - [iOS: framework'ü link etme](#34-ios-frameworkü-link-etme)
   - [Flutter](#35-flutter)
   - [Doğrudan Swift ile yazılmış uygulamalar](#36-doğrudan-swift-ile-yazılmış-uygulamalar)
4. [Build alma](#4-build-alma)
5. [Konfigürasyon](#5-konfigürasyon)
6. [Debug, release ve CI](#6-debug-release-ve-ci)
7. [Binary doğrulama](#7-binary-doğrulama)
8. [Lisanslama](#8-lisanslama)
   - [Anahtarı verme](#81-anahtarı-verme)
   - [Geçerli anahtar yoksa](#82-geçerli-anahtar-yoksa-ne-olur)
   - [Yenileme ve değiştirme](#83-yenileme-ve-değiştirme)
9. [App Store'a gönderim](#9-app-storea-gönderim)
10. [Attestation](#10-attestation)
   - [Uygulamanıza ekleme](#103-uygulamanıza-ekleme)
   - [Backend'de karar verme](#104-backendde-karar-verme)
   - [Servisi çalıştırma](#107-servisi-çalıştırma)
11. [Mevcut bir projeye geçiş](#11-mevcut-bir-projeye-geçiş)
12. [Sorun giderme](#12-sorun-giderme)
13. [SSS](#13-sss)

## 1. Gereksinimler

| Araç | Sürüm |
|---|---|
| JDK | 17–24 (21 önerilir). **25/26 olmaz**: Gradle 8.14, Java 24'ün üstünü reddeder. |
| Gradle | 8.x (8.14 ile test edildi) |
| Kotlin | **2.2.0**; plugin'in compiler build'iyle birebir aynı olmalı |
| AGP | 8.x (Android) |

Compiler plugin'i Kotlin sürümüne bağlıdır. Projeniz Kotlin 2.2.0'da değilse
başlamadan önce sürümü eşitleyin ya da Kotlin sürümünüze uygun bir StringGuard
build'ini bekleyin.

Obfuscate-all varsayılandır. Literal'leri dahil etmezsiniz, hariç tutarsınız.
Compile-time sabiti olarak kalması gereken literal'lere (`const val`
initializer'ları, annotation argümanları) hiç dokunulmaz; bu yüzden plugin'i
tüm projede açmak onlara bağlı kodu bozmaz.

## 2. Kurulum

### 2.1 Repository tanımı

1. Projenizin `settings.gradle.kts` dosyasında repository'yi **iki bloğa da**
   ekleyin:

   ```kotlin
   pluginManagement {
       repositories {
           gradlePluginPortal()
           google()
           mavenCentral()
           maven { url = uri("https://duranun.github.io/stringguard/maven") }
       }
   }
   dependencyResolutionManagement {
       repositories {
           google()
           mavenCentral()
           maven { url = uri("https://duranun.github.io/stringguard/maven") }
       }
   }
   ```

   Credential gerekmez.

2. Projeyi sync edin. Gradle plugin'in kendisini `pluginManagement` üzerinden,
   plugin'in sizin için eklediği `annotations` ve `runtime` kütüphanelerini de
   `dependencyResolutionManagement` üzerinden çeker. İki blokta da tanım
   olmalı; birini unutursanız [bölüm 12](#12-sorun-giderme)'deki resolution
   hatalarından birini alırsınız.

## 3. StringGuard'ı ekleme

Projenizi aşağıda bulun. 3.1–3.4 Kotlin projeleri içindir ve Gradle plugin'ini
kullanır; 3.5 ve 3.6 Flutter ve saf Swift içindir, onların kurulumu farklıdır.

| Projeniz | Bölüm | Literal'ler nasıl korunur |
|---|---|---|
| Multiplatform olmayan Android uygulaması veya kütüphanesi | [3.1](#31-klasik-android-kmp-olmayan) | Gradle plugin'i, modüldeki tüm literal'ler |
| JVM veya Desktop | [3.2](#32-jvm--desktop) | Gradle plugin'i, modüldeki tüm literal'ler |
| Kotlin Multiplatform veya Compose Multiplatform (Android, iOS, JVM, JS, Wasm) | [3.3](#33-kotlin-multiplatform--compose-multiplatform) | Gradle plugin'i, tüm literal'ler, tüm target'lar |
| Kotlin framework'ü kullanan bir iOS uygulaması | [3.3](#33-kotlin-multiplatform--compose-multiplatform) + [3.4](#34-ios-frameworkü-link-etme) | 3.3'teki gibi; 3.4 opsiyonel guard ve attestation için |
| Flutter veya Dart | [3.5](#35-flutter) | Bir secrets dosyasından çalışan code generator |
| Kotlin olmadan doğrudan Swift | [3.6](#36-doğrudan-swift-ile-yazılmış-uygulamalar) | Kaynak kodunuzu yeniden yazan bir Xcode build adımı |

Kotlin bölümleri için: StringGuard'ı modülün zaten kullandığı Kotlin plugin'inin
yanına ekleyin. Sıra önemli değil; StringGuard, Kotlin plugin'i ne zaman
uygulanırsa ona bağlanır. `annotations` veya `runtime`'ı elle eklemeyin; plugin
onları doğru configuration'a veya source set'e kendisi ekler.

### 3.1 Klasik Android (KMP olmayan)

```kotlin
// app/build.gradle.kts
plugins {
    id("com.android.application")      // veya com.android.library
    kotlin("android")
    id("com.stringguard") version "1.0.0"
}

android {
    namespace = "com.example.app"
    compileSdk = 35
    defaultConfig { minSdk = 24 }
    buildTypes {
        // Önerilir. StringGuard iki durumda da çalışır, aşağıdaki nota bakın.
        release { isMinifyEnabled = true }
    }
}

stringGuard {
    enabled = true
    minLength = 4
    licenseKey = providers.environmentVariable("STRINGGUARD_LICENSE").getOrElse("")
}
```

Dependency'ler `implementation` configuration'ına eklenir.

**Release build'lerde minification'ı açık bırakın.** StringGuard'ın R8'e
ihtiyacı yok, onsuz da çalışır; ama R8 runtime korumasının iç sembollerinin
isimlerini değiştirir ve StringGuard buna izin veren kuralları kendisi getirir.
Minification olmadan alınan bir release'te bu isimler APK'da okunabilir kalır ve
decompile eden birine kontrollerin tam olarak nerede olduğunu gösterir.

### 3.2 JVM / Desktop

```kotlin
// build.gradle.kts
plugins {
    kotlin("jvm") version "2.2.0"
    id("com.stringguard") version "1.0.0"
}

stringGuard {
    enabled = true
    minLength = 4
    licenseKey = providers.environmentVariable("STRINGGUARD_LICENSE").getOrElse("")
}
```

Dependency'ler `implementation` configuration'ına eklenir.

### 3.3 Kotlin Multiplatform / Compose Multiplatform

```kotlin
// build.gradle.kts
plugins {
    kotlin("multiplatform") version "2.2.0"
    id("com.stringguard") version "1.0.0"
}

kotlin {
    androidTarget()
    jvm()
    iosArm64(); iosSimulatorArm64()
    js(IR) { nodejs() }
    wasmJs { nodejs() }
    // ...
}

stringGuard {
    enabled = true
    minLength = 4
    licenseKey = providers.environmentVariable("STRINGGUARD_LICENSE").getOrElse("")
}
```

`annotations` ve `runtime` dependency'leri `commonMain`'e eklenir; Apple ve Wasm
dahil tüm target'lar aynı konfigürasyonla kapsanır. Target bazında ayrı bir ayar
gerekmez. Her target aynı şekilde obfuscate edilir: tek lisans ve tek özellik
seti olduğu için Apple, JS ve Wasm target'ları JVM ve Android'den farklı
muamele görmez.

**JS ve Wasm** için yukarıdakinden fazlası gerekmez. Korunan bir değer `js(IR)`
ve `wasmJs`'te JVM'dekiyle birebir aynı çözülür. Browser ve Node target'ları
aynı davranır.

**Compose Multiplatform** bu bölümle olduğu gibi kapsanır: o da bir
Multiplatform projesidir ve plugin Compose'a özel bir şey yapmaz. Composable
fonksiyonların içindeki literal'ler de diğerleri gibi obfuscate edilir.

### 3.4 iOS: framework'ü link etme

iOS target'ında string obfuscation için 3.3'ten fazlası gerekmez. Bu bölüm,
Swift'e bir Kotlin framework'ü üzerinden ulaşan opsiyonel runtime guard ve
attestation client'ı içindir.

**Mevcut bir multiplatform uygulamaya eklemeden önce okuyun.** Bir iOS
uygulaması iki Kotlin framework'ünü birlikte link edemez. Bu yüzden
`StringGuard.framework`, runtime guard'ı ve attestation client'ı tek bir
framework'te taşır.

`StringGuard.xcframework` lisansınızla birlikte dağıtılır; nereden alacağınız
lisansınızda yazar. Device ve simulator slice'larını, debug sembolleriyle
birlikte içerir.

Uygulamanızın kendi paylaşılan Kotlin framework'ü **yoksa**, onu normal yoldan
link edin: CocoaPods (`vendored_frameworks`), Swift Package Manager
(`.binaryTarget`) veya elle embed etmek, hepsi olur.

Uygulamanız **zaten kendi paylaşılan Kotlin framework'ünü ship ediyorsa**,
`StringGuard.framework`'ü onun yanına link etmeyin. Kütüphaneleri kendi
framework'ünüze ekleyin:

```kotlin
// mevcut shared modülünüz
kotlin {
    sourceSets.commonMain.dependencies {
        api("com.stringguard:guard:<version>")        // api, implementation DEĞİL
        api("com.stringguard:attest-core:<version>")
    }
    listOf(iosArm64(), iosSimulatorArm64(), iosX64()).forEach {
        it.binaries.framework {
            baseName = "Shared"
            export("com.stringguard:guard:<version>")
            export("com.stringguard:attest-core:<version>")
        }
    }
}
```

`implementation` değil `api` olmalı: yalnızca `api` dependency'leri export
edilebilir, export edilmezse semboller oluşturulan header'lara ulaşmaz.

Hiç Kotlin olmadan **doğrudan Swift** ile yazılmış bir uygulama için bunun
yerine [bölüm 3.6](#36-doğrudan-swift-ile-yazılmış-uygulamalar)'ya bakın. O
yolun kurulumu farklıdır; kendi varsayılanları ve hariç tutma söz dizimi vardır.

### 3.5 Flutter

Flutter'da birbirinden ayrı iki parça var. İkisinden birini ya da ikisini
birden kullanabilirsiniz:

| İstediğiniz | Package'lar | Nasıl çalışır |
|---|---|---|
| **String obfuscation** | `stringguard` (`dependencies`) ve `stringguard_generator` (`dev_dependencies`) | Secret'larınız kaynak kodunuzun dışındaki bir dosyada durur. Build sırasında bir code generator bunları Dart kodunuzun okuduğu obfuscate edilmiş değerlere çevirir. Kurulumu aşağıda. |
| **Runtime guard ve attestation** | `stringguard_flutter` (`dependencies`) | StringGuard'ın native kontrollerini Android ve iOS'ta çalıştıran bir plugin. Attestation kurulumu [10.3.1](#1031-flutter)'de. |

Kotlin plugin'inden farklı olarak Flutter tarafı Dart kodunuzda zaten yazılı
olan string literal'leri **obfuscate etmez**. Yalnızca secrets dosyasından
tanımlanan değerler korunur; her secret'ı oraya taşıyın.

#### Obfuscation

`pubspec.yaml` dosyanızda (package'ları nereden alacağınız lisansınızda yazar,
pub.dev'de değiller):

```yaml
dependencies:
  stringguard: <lisansınızda yazan>

dev_dependencies:
  stringguard_generator: <lisansınızda yazan>
  build_runner: ^2.4.0
```

Secret'ları tanımlayın. `secrets.env` dosyasını version control'e koymayın:

```dart
import 'package:stringguard/annotations.dart';
import 'package:stringguard/stringguard.dart';

part 'config.g.dart';

@StringGuardEnv('secrets.env')
class Config {
  @Secret()                       // alan adıyla aranır
  static final String apiKey = _$Config.apiKey;

  @Secret('API_BASE_URL')         // ya da açıkça verilen bir key ile
  static final String baseUrl = _$Config.baseUrl;
}
```

Sonra generate edin:

```bash
dart run build_runner build      # geliştirirken `watch`
```

Erişim her zaman `_$<ClassName>.<fieldName>` şeklindedir ve oluşturulan part
dosyasında tanımlanır. Generator'ı ilk compile'dan önce çalıştırın, yoksa part
dosyası henüz yoktur. Builder ayarı gerekmez: generator, ona bağımlı her
package'a otomatik uygulanır.

Runtime guard veya attestation için `stringguard_flutter` eklerseniz, kendi iOS
privacy manifest'iyle geldiğini unutmayın; bkz.
[bölüm 9](#9-app-storea-gönderim).

### 3.6 Doğrudan Swift ile yazılmış uygulamalar

Hiç Kotlin içermeyen bir uygulama için ayrı bir entegrasyon, Swift kaynak
kodunuzu Xcode build'i sırasında yeniden yazar; siz normal string literal'ler
yazmaya devam edersiniz.

Üç adım, ayrıntısı [swift/README.md](../swift/README.md)'de:

1. Swift package'ı ekleyip `StringGuardRuntime` product'ını link edin.
2. Compile Sources'tan **önce** bir **Run Script** build phase'i ekleyin; bu
   adım kaynaklarınızı build dizinine yeniden yazar ve bir virtual-filesystem
   overlay'i oluşturur.
3. Build Settings'te **Other Swift Flags** ile compiler'ı bu overlay'e
   yönlendirin: `-vfsoverlay $(DERIVED_FILE_DIR)/stringguard/overlay.yaml`.

Kendi kaynak dosyalarınız hiç değiştirilmez; compiler yeniden yazılmış
kopyaları okur.

**Bu yolun ayarları Kotlin'inkinden farklıdır; karışık bir projede tek bir
policy'nin ikisini de kapsadığını varsaymayın.** Yapmanız gerekeni değiştiren
farklar:

| | Kotlin | Swift |
|---|---|---|
| Minimum literal uzunluğu | `minLength`, varsayılan 1 | `--min-length` flag'i, varsayılan **4** |
| Hariç tutma | `@DontObfuscate` / `@KeepPlain`, `keepRules` | `// stringguard:keep` yorumu ya da hariç tutulan bir path |
| Interpolation içeren literal'ler | obfuscate edilir | **yeniden yazılmaz** |
| Çok satırlı ve raw literal'ler | obfuscate edilir | **yeniden yazılmaz** |

Plan yaparken dikkat edilecek olan interpolation: interpolation ile oluşturulan
bir secret okunabilir kalır. Bu yolda secret'ları tek parça literal olarak
tutun. Platform minimumları ve hiç yeniden yazılmayanların tam listesi
[swift/README.md](../swift/README.md)'de.

## 4. Build alma

Projeyi her zamanki gibi build edin:

```bash
# Android
./gradlew assembleRelease

# JVM / KMP
./gradlew build
```

Kaynak kodunuz değişmez. Normal string literal'ler yazmaya devam edersiniz:

```kotlin
object Config {
    val apiBaseUrl = "https://api.example.com/v2"   // release binary'sinde obfuscate edilir
    val analyticsKey = "ak_prod_1a2b3c4d5e"         // obfuscate edilir
}
```

Runtime'da değerler yazdığınızın aynısıdır; binary'de ise yoktur. Bunu
[bölüm 7](#7-binary-doğrulama)'deki kontrollerle görebilirsiniz.

## 5. Konfigürasyon

`stringGuard { }` bloğu aşağıdaki ayarları alır. Ayarlanmayanlar için
varsayılan geçerlidir.

| Ayar | Varsayılan | Etkisi |
|---|---|---|
| `enabled` | `true` | Ana şalter. false ise compiler plugin'i hiçbir şey yapmaz. |
| `obfuscateAll` | `true` | Proje genelinde uygun tüm literal'leri obfuscate eder. false ise yalnızca `@Obfuscate` kapsamları. |
| `minLength` | `1` | Bu kadar karakterden kısa literal'ler plain kalır. |
| `enabledForDebug` | `false` | Debug build type'larını da obfuscate eder. Test compilation'ları her durumda atlanır. |
| `algorithm` | `StringGuardAlgorithm.AES_GCM` | AEAD algoritması. Alternatifi `StringGuardAlgorithm.CHACHA20_POLY1305`. |
| `licenseKey` | `""` | **Zorunlu.** Offline lisans token'ı ya da modül dizinine göre bir lisans dosyası yolu. Eksik, bozuk veya süresi dolmuşsa build başarısız olur. Bkz. [bölüm 8](#8-lisanslama). |
| `keepRules { }` | kural yok | Package ve regex bazlı hariç tutmalar, aşağıda. |

### `@DontObfuscate` ile kapsam dışı bırakma

Bir dosyayı, class'ı, object'i, fonksiyonu, property'yi veya constructor'ı
işaretlerseniz içindeki tüm literal'ler okunabilir kalır:

```kotlin
import com.stringguard.annotations.DontObfuscate

@DontObfuscate                        // dosya, class, object, fonksiyon, property veya constructor
object ReflectionKeys {
    const val COLUMN = "user_id"      // (const val zaten atlanırdı)
    val lookupName = "com.example.Thing"   // Class.forName ile okunuyor, okunabilir kalmalı
}
```

`@KeepPlain` aynı etkiye sahiptir. Tek fark annotation target'larındadır:
`@DontObfuscate` bütün bir dosyaya uygulanabilir, `@KeepPlain` uygulanamaz.

### `keepRules` ile pattern bazlı hariç tutma

```kotlin
stringGuard {
    minLength = 4
    keepRules {
        keepPackage("com.example.resources.*")   // package ve tüm alt package'lar plain kalır
        keepMatching("^https://cdn\\.")           // regex'e uyan literal'ler plain kalır
        keepMatching("^android\\.intent\\.")      // örn. sistem intent action string'leri
    }
}
```

`keepPackage("a.b.*")`, `a.b` package'ını ve altındaki tüm package'ları tutar.
`keepMatching(regex)`, içeriği regex'e uyan (`containsMatchIn`) her literal'i
tutar.

### Hiçbir zaman obfuscate edilmeyen literal'ler

Plugin aşağıdakileri her zaman compile-time sabiti olarak bırakır, çünkü
obfuscate edilseler kod derlenmez. Bunlar için kural gerekmez.

- `const val` initializer'ları
- Annotation argümanları (`@SerialName("…")`, `@GET("…")`, `@Deprecated("…")`, …)
- Annotation class default değerleri

Retrofit, kotlinx.serialization ve Room'un obfuscate-all açıkken çalışmaya
devam etmesinin sebebi bu.

### Opt-in modu

Yalnızca açıkça işaretlenen kapsamları obfuscate etmek için proje geneli
obfuscation'ı kapatın:

```kotlin
stringGuard { obfuscateAll = false }
```

Sonra istediğiniz kapsamları işaretleyin:

```kotlin
import com.stringguard.annotations.Obfuscate

@Obfuscate
class PaymentClient {
    private val merchantSecret = "msk_live_..."   // obfuscate edilir
}
// @Obfuscate kapsamları dışındaki her şey plain kalır
```

## 6. Debug, release ve CI

Hangi compilation'ların obfuscate edileceğini üç kural belirler:

- Test compilation'ları her zaman atlanır, hiçbir zaman obfuscate edilmez.
  Kotlin ve AGP'nin `test`, `testFixtures` ya da sonu `Test` ile biten
  isimlerle adlandırdığı compilation'lar: `jvmTest`, `debugUnitTest`,
  `freeDebugAndroidTest`.
- Debug build type'ları `enabledForDebug = true` olmadıkça plain kalır. Buna
  `debug` ve flavor'lı tüm debug variant'ları (`prodDebug`, `freeDebug`) dahil.
- Geri kalan her şey obfuscate edilir; `staging` veya `qa` gibi özel build
  type'ları da. `enabledForDebug` bunları plain bırakamaz; keep rule kullanın.

Sınıflandırma tam isim ve sona göre yapılır, isim içinde geçmesine göre değil.
İsmi yalnızca "test" ya da "debug" içeren bir product flavor (`latest`,
`contest`, `fastest`) test veya debug compilation'ı sanılmaz.

```kotlin
stringGuard {
    enabledForDebug = false   // dev build'leri hızlı, log'ları grep'lenebilir tut
}
```

Debug build'ler varsayılan olarak plain kalır, çünkü obfuscation her literal
için küçük bir maliyet ekler ve geliştirme sırasında `strings` çıktısını ve
logcat'i okumayı zorlaştırır. Korunması gereken, ship edilen konfigürasyondur.

CI'da lisans ve sabitlenmiş bir JDK yeterli, başka bir şey gerekmez. Tipik bir
release pipeline'ı:

```bash
export STRINGGUARD_LICENSE="<CI secret store'unuzdan inject edilir>"
./gradlew clean assembleRelease      # veya :app:bundleRelease
```

Token'ı CI secret'ı olarak inject edin, asla commit etmeyin. Token yoksa build
başarısız olur; onu unutan bir pipeline sessizce korumasız string'ler ship
edemez.

CI, JDK 25 veya 26 içeren bir container image kullanıyorsa 17–24 aralığında bir
JDK sabitleyin; bkz. [bölüm 12](#12-sorun-giderme). Sonraki bölümdeki plain text
sızıntı kontrolünü build gate olarak ekleyin.

## 7. Binary doğrulama

Korunan literal'ler derlenmiş çıktıda yer almaz. Her StringGuard sürümü, bu
desteklenen her backend'de gösterilmeden yayınlanmaz; yani bunu sizin ayrıca
kanıtlamanız gerekmez.

Kendi build'inizde görmek isterseniz en hızlı yer JVM çıktısıdır:

```bash
./gradlew build
javap -c -p -classpath build/libs/<your>.jar com.example.Config
# secret literal'ler yok
```

Derlenmiş bir binary üzerinde düz metin araması her platformda güvenilir bir
kontrol değildir; sonucunu kesin değil, belirsiz kabul edin.

## 8. Lisanslama

Lisans **zorunludur**. StringGuard'ın tek bir özellik seti ve tek bir anahtar
türü var: geçerli bir lisans her şeyi tüm target'larda açar; free, trial ya da
ara bir seviye yoktur.

Doğrulama offline yapılır. Token, plugin'e gömülü bir public key ile
configuration aşamasında, ağa çıkmadan kontrol edilir; CI'da ve internete kapalı
ortamlarda da çalışır.

### 8.1 Anahtarı verme

```kotlin
stringGuard {
    licenseKey = "<token>"      // ya da bir lisans dosyasının yolu (modüle göre)
}
```

Token gizlidir, version control'e koymayın. Genelde environment'tan okunur; bu
aynı zamanda onu Gradle configuration cache kaydının dışında tutar:

```kotlin
stringGuard {
    licenseKey = providers.environmentVariable("STRINGGUARD_LICENSE")
        .orElse(providers.gradleProperty("stringguard.license"))
        .getOrElse("")
}
```

CI'da secret olarak inject edin. Dosya da olur; bir secret manager'ın mount
ettiği anahtar için uygundur:

```kotlin
stringGuard { licenseKey = "config/stringguard.license" }
```

### 8.2 Geçerli anahtar yoksa ne olur

**Build başarısız olur.** Configuration, istediğinizden daha az koruyan bir
build üretmek yerine açıklayıcı bir hatayla durur:

| Durum | Sonuç |
|---|---|
| `licenseKey` ayarlı değil, boş ya da boş bir dosya | `StringGuard requires a license key. ...` |
| StringGuard token'ı değil, eksik kopyalanmış ya da başka bir key ile imzalanmış | `StringGuard license is not valid: <reason>. ...` |
| Eski v1 formatında verilmiş | `... license format v1 is no longer supported; reissue the key ...` |
| Son geçerlilik günü geçmiş | `StringGuard license '<id>' expired on <date>. ...` |

Bunu hata olmaktan çıkaran bir flag yoktur. Alternatifi, yani devam edip daha
az obfuscate etmek, yeşil bir build'den okunabilir secret'lar içeren bir binary
ship etmek demektir; bir string obfuscator'ın asla yol açmaması gereken tek
sonuç budur.

Gerçekten obfuscate edilmemesi gereken bir modül kapsam dışında kalır, o zaman
anahtar gerekmez:

```kotlin
stringGuard { enabled = false }
```

Başarılı bir build lisansı `lifecycle` seviyesinde bir kez log'lar:

```
StringGuard: license verified (holder=Acme Inc, expires=2027-04-01)
```

Gradle **configuration cache hit** durumunda proje yeniden configure edilmez;
lisans tekrar doğrulanmaz ve bu satır yazılmaz. İki cache'li build arasında
süresi dolan bir lisans bir sonraki soğuk configuration'da yakalanır.

### 8.3 Yenileme ve değiştirme

Lisans belirli bir süre için verilir; yenilemek, build'inizin okuduğu token'ı
değiştirmek demektir. Token'ı [8.1](#81-anahtarı-verme)'deki gibi
environment'tan veya bir dosyadan veriyorsanız yenileme yalnızca bir secret
store değişikliğidir; kod değişikliği ya da release gerekmez.

Yenilemeyi pay bırakarak planlayın. Doğrulama build'i durdurduğu için süresi
dolmuş bir anahtar, bir olay sırasındaki hotfix build'i dahil tüm build'leri
durdurur. Lisansa sorumlu birini atayın, son tarihten haftalar önce bir alarm
kurun ve yenileme yolunu nöbetçi mühendisin bulabileceği bir yerde tutun.

Lisans **son geçerlilik tarihi dahil** geçerlidir ve tarih hem lisansı veren
taraf hem build tarafından **UTC** olarak değerlendirilir. Yani 2027-04-01
tarihli bir anahtar agent'larınız nerede çalışırsa çalışsın 2027-04-01'de hâlâ
build alır, 2027-04-02'de durur. Yine de pay bırakın; yenilemeyi tam sınır
gününe planlamayın.

Yayınlanmış uygulamalar etkilenmez: zaten ship ettiğiniz runtime bileşenleri
son kullanıcılarınız için lisanslı kalır. Yalnızca yeni build alabilmeniz durur.

Yeni bir ekip, farklı bir build sistemi ya da sızıntı sonrası değişim için
anahtara ihtiyacınız varsa her zamanki ticari kontağınız üzerinden isteyin.
Sızan bir token'a diğer credential'lar gibi davranın: yenisini alın ve onu
tutan her pipeline'da secret'ı değiştirin.

## 9. App Store'a gönderim

Bir iOS uygulamasına StringGuard eklemek iki App Store yükümlülüğü doğurur. İkisi
de uygulamayı gönderen geliştiricinin cevaplaması gereken sorulardır ve
varsayılanlarla değil, gerçeklerle cevaplanır.

### Encryption export compliance

StringGuard korunan literal'leri AES-GCM ile çözer. Apple target'larında bu
decrypt işlemini CryptoKit ya da başka bir işletim sistemi framework'ü değil,
StringGuard'ın kendi implementasyonu yapar.

Apple'ın rehberi, `URLSession` üzerinden HTTPS gibi işletim sistemine gömülü
şifrelemeyi genelde export dokümantasyonu gerekliliğinden muaf sayar; uygulamanın
içinde gelen bir implementasyonu ise bu kapsamda saymaz. Dolayısıyla App Store
Connect'teki şifreleme sorusunun cevabı ve `Info.plist`'teki
`ITSAppUsesNonExemptEncryption` değeri, StringGuard eklenmeden önceki halinden
farklı olabilir. Muaf olmayan bir cevap, yıl sonu self-classification raporu
gerektirebilir.

Bu doküman uygulamanız için hangi cevabın doğru olduğunu söylemez; bu sizin
bulunduğunuz ülkeye ve diğer dependency'lerinize bağlıdır. Cevaplamak için
gereken bilgiyi verir: şifreleme işletim sisteminde değil, uygulamanın
içindedir. Bkz.
[Complying with Encryption Export Regulations](https://developer.apple.com/documentation/security/complying-with-encryption-export-regulations).

### Privacy manifest

Gradle plugin'i, compiler plugin'i ve obfuscation runtime'ı hiçbir
required-reason API kullanmaz; yani tek başına string obfuscation privacy
manifest'inize hiçbir şey eklemez.

**Runtime guard farklıdır.** Jailbreak kontrolü, uygulama container'ı dışındaki
path'lerin dosya metadata'sını okur; bu, Apple'ın **file-timestamp**
required-reason API kategorisine girer. Runtime guard'ı
`StringGuard.framework`'ü link ederek ya da guard kütüphanesini
[bölüm 3.4](#34-ios-frameworkü-link-etme)'teki gibi kendi framework'ünüzden
export ederek açarsanız, uygulamanızın `PrivacyInfo.xcprivacy` dosyasında bu
kategori için bir `NSPrivacyAccessedAPITypes` kaydı olmalıdır.

Göndermeden önce güncel reason-code önerisini bizden isteyin. Apple
required-reason listesini güncelliyor ve doğru tanım StringGuard'ın yanı sıra
uygulamanıza da bağlı; bu yüzden kodu bu dokümana sabitlemek yerine öneriyi
güncel tutuyoruz. Tanımlanmamış bir required-reason API genelde reddedilmeye
değil, yüklemede bir `ITMS-91053` uyarısına yol açar; göndermeden önce
halletmeniz gereken bir şeydir ama engel değildir.

Flutter plugin'inin durumu ayrı: `UserDefaults`'ta tek bir değeri, App Attest
key identifier'ını okuyup yazar. `UserDefaults` bir required-reason API'dir ve
tanımlanmamış kullanımı yüklemede `ITMS-91053` uyarısına yol açar. Plugin kendi
`PrivacyInfo.xcprivacy` dosyasıyla gelir ve
`NSPrivacyAccessedAPICategoryUserDefaults` kategorisini `CA92.1` reason'ıyla
tanımlar; uygulamanızın manifest'inde bunun için bir şey eklemeniz gerekmez.
Bkz.
[Describing use of required reason API](https://developer.apple.com/documentation/bundleresources/privacy_manifest_files/describing-use-of-required-reason-api).

## 10. Attestation

Opsiyonel. Attestation, her uygulama session'ı için backend'inize isteğin sağlam
bir cihazdaki gerçek uygulamanızdan gelip gelmediğini söyler. Uygulama kendini,
kendi altyapınızda çalıştırdığınız attestation servisine kontrol ettirir;
sonucu backend'iniz okur ve neye izin vereceğine kendisi karar verir.

### 10.1 Sonuçlar

| Sonuç | Ne zaman çıkar |
|---|---|
| `ALLOW` | Gerçek uygulamanız, gerçek olduğunu kanıtlayan bir cihazda. |
| `ALLOW_LIMITED` | Uygulamanız, gerçek olduğunu kanıtlayamayan bir cihazda. Eski cihazlarda ve emulator'lerde sık görülür, genelde saldırı değildir. |
| `FLAG` | Uygulamanız, ama cihazda kurcalama izleri var: debugger, hooking framework, root'lu cihaz ya da değiştirilmiş sistem. |
| `DENY` | Gerçek uygulamanız değil ya da kontrol başarısız. Session token verilmez. |

Akılda tutun:

- **Sonuç, cihazın kontrol edildiği andaki durumudur.** Uygulama her session'da
  tekrar kontrol eder.
- **Attestation, obfuscate edilmiş string'lerin korunma şeklini değiştirmez.**
  Backend'inize cihaz hakkında bilgi verir; cihazdaki değerleri daha güvenli
  yapmaz.

### 10.2 Platform gereksinimleri

| | Gereksinim |
|---|---|
| Android | `ALLOW` sonucu için API 26+. Daha eski cihazlar `ALLOW_LIMITED` alır. |
| iOS | Apple App Attest; uygulamanın provisioning profile'ında **App Attest capability**'si ve **fiziksel cihaz** gerekir. Simulator ve Secure Enclave olmayan cihazlar hata vermek yerine "unsupported" döner. |
| Diğer target'lar | Desteklenmez. Çağrılar hata vermek yerine "unsupported" döner. |

Unsupported bir hata değil, normal bir durumdur: her client çağrısı
inceleyebileceğiniz bir sonuç döner, hiçbiri exception fırlatmaz.

### 10.3 Uygulamanıza ekleme

Client kütüphanesini shared modülünüze ekleyin:

```kotlin
// commonMain
implementation("com.stringguard:attest-core:<version>")
```

iOS'ta uygulamanız zaten kendi Kotlin framework'ünü link ediyorsa önce
[bölüm 3.4](#34-ios-frameworkü-link-etme)'ü okuyun; iki tane link edemez.

```kotlin
// 1. Servisinizden bir nonce alın, sonra attest edin. Sonraki request'leri
//    imzalayabilmek için key'i saklayın. Apple'da suspend versiyonu kullanın:
//    App Attest asenkrondur, senkron bir yolu yoktur.
val alias = deviceKeyAlias(tenantId)          // SDK dağıtıyorsanız bkz. 10.6
val outcome = platformAttestPersistent(nonce, alias)

// 2. Runtime guard'ın bulgularını da ekleyin.
val r = Guard.report()
val signals = DeviceSignals(r.debuggerAttached, r.rootDetected, r.hookingDetected)
val statement = buildAttestationStatement(nonce, signals, System.currentTimeMillis())

// 3. Statement'ı servisinize gönderin; size bir session token döner.

// 4. Her hassas request'i attest edilmiş key'e bağlayın.
val envelope = signRequest(
    alias, sessionToken, "POST", "/secret", bodySha256,
    System.currentTimeMillis(), freshRandomNonce,
)
```

Request binding (4. adım) Android'de vardır; API'nizin bunu nasıl kontrol
edeceği [10.4.4](#1044-requesti-cihaza-bağlama)'te. iOS'ta ihtiyacınız varsa
tasarımınızı buna göre kurmadan önce bize iletin.

#### 10.3.1 Flutter

`stringguard_flutter`'ı `dependencies`'e ekleyin (bkz. [3.5](#35-flutter)).
`StringGuardClient` kontrolün tamamını tek çağrıda yapar:

```dart
import 'package:stringguard_flutter/stringguard_flutter.dart';

final sg = StringGuardClient(
  baseUrl: 'https://attest.example.com',   // attestation servisiniz
  licenseToken: licenceToken,
  tenantId: 'your-app',
);

// Açılışta ve session'ın süresi dolunca tekrar:
final session = await sg.establishSession(deviceId: deviceId);
final token = session.sessionToken;   // cihaz engellendiyse null
// `token`'ı her request'te kendi API'nize gönderin (10.4).

// Hassas bir request için (yalnızca Android; diğerlerinde null):
final signed = await sg.signedRequest(
  sessionToken: token!, method: 'POST', path: '/payments', body: bodyBytes,
);
```

Request body olarak tam olarak `bodyBytes`'ı gönderin ve `signedRequest`'in
döndürdüğü değerleri [10.4.4](#1044-requesti-cihaza-bağlama)'teki gibi taşıyın:
`timestamp` → `X-Request-Timestamp`, `requestNonce` → `X-Request-Nonce`,
`signature` → `X-Request-Signature`. `null` dönerse cihaz imzalayamıyor demektir;
request'i bağlanmamış kabul edin.

Attestation olmadan yalnızca runtime guard:

```dart
final report = await StringguardFlutter().guardReport();
if (report.suspicious) { /* kararınız */ }
```

### 10.4 Backend'de karar verme

```
Uygulamanız ──► attestation servisi ──► session token
Uygulamanız ──► kendi API'niz, her request'te session token ile
API'niz     ──► token'ı offline doğrular, kararı okur, karar verir
```

Attestation servisi hiçbir zaman uygulamanız ile API'niz arasında durmaz.
Cihazı kontrol eder, sonucu imzalı bir session token'a yazar ve uygulamaya
verir. API'niz token'ı servise geri sormadan kendisi doğrular ve request'in ne
yapabileceğine karar verir.

#### 10.4.1 Session token'ı doğrulama

Session token, **Ed25519 ile imzalanmış** (`alg: EdDSA`) standart bir **JWT**'dir.
Backend'inizin zaten kullandığı JWT kütüphanesiyle doğrulayın; sunucuda
StringGuard'dan hiçbir şeye ihtiyacınız yok.

Doğrulama key'lerini attestation servisiniz bir JSON Web Key Set olarak
yayınlar:

```
https://<attestation servisiniz>/.well-known/jwks.json
```

Key'leri kopyalamak yerine kütüphanenizin key set desteğiyle buradan çekin:
session key'lerini değiştirdiğinizde yeni key burada görünür ve backend'iniz
hiçbir değişiklik yapmadan onu alır. Kütüphaneyi şöyle ayarlayın:

- **yalnızca** `EdDSA` algoritmasını kabul etsin,
- issuer olarak **`stringguard-attest`** istesin,
- süre dolumunu kontrol etsin (çoğu kütüphane varsayılan olarak yapar),
- aynı attestation servisini birden fazla uygulamanız kullanıyorsa `lic`
  claim'inin bu API'nin hizmet verdiği lisans olmasını istesin. Aksi halde bir
  uygulamaya verilen token, başka bir uygulamanın API'sinde de kabul edilir.

Key set'i HTTPS üzerinden ya da güvendiğiniz bir ağdan çekin. Attestation
servisi ingress'inizin arkasında plain HTTP konuşur; key set'i yolda
değiştirebilen biri backend'inizin kabul edeceği token'lar üretebilir.

Node.js, [`jose`](https://github.com/panva/jose) ile:

```js
import { createRemoteJWKSet, jwtVerify } from 'jose';

const keys = createRemoteJWKSet(new URL('https://attest.example.com/.well-known/jwks.json'));

const { payload } = await jwtVerify(token, keys, {
  issuer: 'stringguard-attest',
  algorithms: ['EdDSA'],
});
```

Python, [PyJWT](https://pyjwt.readthedocs.io/) ile (`pip install "pyjwt[crypto]"`):

```python
import jwt

keys = jwt.PyJWKClient("https://attest.example.com/.well-known/jwks.json")

claims = jwt.decode(
    token,
    keys.get_signing_key_from_jwt(token).key,
    algorithms=["EdDSA"],
    issuer="stringguard-attest",
)
```

JWT kütüphanelerinin çoğu EdDSA destekler; Java için `nimbus-jose-jwt`, Go için
`golang-jwt` bunlardan. Endpoint yapınızı kurmadan önce sizinkinin desteklediğini
kontrol edin.

#### 10.4.2 Token'da neler var

| Claim | Anlamı |
|---|---|
| `decision` | Kontrolün sonucu: `ALLOW`, `ALLOW_LIMITED` veya `FLAG`. |
| `sub` | Uygulamanızın verdiği cihaz kimliği. **Bunu cihaz kanıtlamaz, uygulama seçer**; yalnızca `sub`'a dayanarak hiçbir yetki vermeyin. Saldırganın seçemeyeceği bir cihaz kimliği gerekiyorsa `device_key` kullanın. |
| `lic` | Uygulamanın kullandığı lisans. |
| `exp` | Token'ın süresinin dolduğu an, epoch'tan beri saniye olarak. Session'lar kısa ömürlüdür; uygulama yenisi için tekrar kontrol eder. |
| `device_key` | Cihazın attest edilmiş public key'i (base64, SubjectPublicKeyInfo DER). Yalnızca cihaz kendini donanımla kanıtladığında bulunur. [Request binding](#1044-requesti-cihaza-bağlama) için kullanılır. |
| `iss` | Her zaman `stringguard-attest`. |

Engellenen bir cihaza **hiç token verilmez**; doğrulanmış bir token hiçbir zaman
`DENY` taşımaz.

Token `exp`'e kadar geçerlidir. Bir lisansı iptal etmek ya da bir cihazı
kaldırmak uygulamanın bir sonraki kontrolünde etkili olur; hemen öncesinde
verilmiş bir token session süresi boyunca kullanılabilir kalır: daha düşük bir
`SG_SESSION_TTL_MS` ayarlamadıysanız 15 dakika.

#### 10.4.3 Karar verme

Her sonucun neye izin vereceği sizin kararınız; önerimiz
[10.5](#105-önerilen-policy)'te. Ne seçerseniz seçin tek bir kurala uyun:

> **Geçerli token'ı olmayan bir request, en az güvenilen token'ı taşıyan bir
> request'ten asla daha fazla yetki almamalıdır.**

Token'ı uygulamanız gönderir, yani saldırgan onu basitçe göndermeyebilir.
`FLAG` alan bir cihaz ödemede reddediliyor ama token'sız bir request'e ödeme
izni veriliyorsa, saldırı token'ı göndermeyi bırakmaktır. Engellenen bir
cihazın zaten token'ı yoktur, tam olarak o durumda gelir. Token'ı olmayan
request'i en az `FLAG` kadar kısıtlayın ve hassas her işlem için geçerli bir
token isteyin.

Tipik bir eşleme:

| API'nize gelen | Anlamı | Tipik yanıt |
|---|---|---|
| Token yok ya da doğrulanmıyor | Cihaz kontrol edilmedi ya da engellendi | Kontrol edilmemiş bir client gibi davranın: yalnızca herkese izin verdiğiniz şeylere izin verin, asla `FLAG`'den fazlasına değil. Hassas işlemleri reddedin. |
| `ALLOW` | Cihaz gerçek olduğunu kanıtladı | Tam erişim. |
| `ALLOW_LIMITED` | Cihaz gerçek donanım olduğunu kanıtlayamadı | Normal kullanım. Genelde saldırı değildir; eski cihazlar ve emulator'ler buraya düşer. En hassas işlemleriniz için step-up düşünün. |
| `FLAG` | Cihazda kurcalama izleri var | Sıradan kullanıma izin verin; ödeme gibi hassas işlemleri reddedin ya da step-up isteyin. |

Karar token'ın içinde olduğu için ona nasıl yanıt vereceğinizi istediğiniz zaman,
uygulama güncellemesi olmadan değiştirebilirsiniz.

#### 10.4.4 Request'i cihaza bağlama

Bir session token, onu alan cihazdan kopyalanabilir. Bunun önemli olduğu
request'ler için uygulama ayrıca **her request'i cihazın attest edilmiş key'iyle
imzalayabilir**; API'niz de imzanın token'daki `device_key` ile eşleştiğini
kontrol eder. Böylece kopyalanan token cihaz olmadan işe yaramaz. Bu, Android'de
cihaz kendini donanımla kanıtladığında, yani token `device_key` taşıdığında
kullanılabilir.

Uygulama her request için `signRequest(...)` ile bir imza üretir (bkz.
[10.3](#103-uygulamanıza-ekleme)) ve request ile birlikte gönderir. Nasıl
taşıyacağınız size kalmış; örneğin:

```
Authorization:       Bearer <session token>
X-Request-Timestamp: 1767225600000          (epoch'tan beri milisaniye)
X-Request-Nonce:     <base64, her request için yeni rastgele bir değer>
X-Request-Signature: <base64>
```

API'niz sonra:

1. **Session token'ı doğrular** ([10.4.1](#1041-session-tokenı-doğrulama)) ve
   `device_key`'ini alır. **Endpoint binding gerektiriyor ve token'da
   `device_key` yoksa request'i reddedin.** Binding'i "uygulanamaz" sayıp
   geçirmeyin: `device_key`'siz token, kendini kanıtlayamayan bir cihazın
   sunduğu şeyin ta kendisidir; onun için kontrolü atlamak bu tür her cihazı
   içeri alır.
2. **Kendi saatinden 60 saniyeden fazla farklı bir timestamp'i reddeder.**
   Header'ı yalnızca karşılaştırmak için parse edin; 3. adım metnini tam olarak
   geldiği gibi kullanır.
3. **İmzalanan byte'ları gerçekte gelen veriden yeniden oluşturur**; body'yi
   parse etmeden, ham byte olarak. Aşağıdaki alanların her biri bu sırayla,
   4 byte big-endian uzunluk ve ardından kendi byte'ları olarak yazılır:

   | # | Alan |
   |---|---|
   | 1 | `sgreq.v1` metni |
   | 2 | session token, tam olarak geldiği gibi |
   | 3 | HTTP method'u, örn. `POST` |
   | 4 | varsa query string dahil path, tam olarak uygulamanın istediği gibi, örn. `/payments?id=7` |
   | 5 | request body'nin SHA-256'sı (32 byte; body yoksa boş body'nin hash'i) |
   | 6 | timestamp, ondalık metin olarak, örn. `1767225600000` |
   | 7 | base64'ten çözülmüş nonce |

4. **İmzayı doğrular**: `device_key`'e karşı, P-256 üzerinde SHA-256 ile ECDSA,
   DER encoded.
5. **Nonce'u kaydeder, zaten kayıtlıysa reddeder**; bunu tek adımda yapın ki
   bir request'in iki kopyası birden geçemesin. Nonce'u yalnızca imza
   doğrulandıktan sonra kaydedin; sahte bir request gerçek bir request'in
   nonce'unu tüketemesin. Kabul edilen nonce'ları iki dakika saklayın, **tüm
   instance'ların ve bağlı request kabul eden tüm API'lerin ortak kullandığı tek
   bir store'da**: imza host'u kapsamaz, ayrı store'larla birinin kabul ettiği
   request diğerine tekrar gönderilebilir.
6. **Tam olarak doğruladığı method, path ve body üzerinde işlem yapar**; onların
   yeniden parse edilmiş ya da yeniden serialize edilmiş hali üzerinde değil.

Node.js'te, yalnızca built-in `crypto` modülüyle:

```js
import crypto from 'node:crypto';

function requestIsFromTheDevice({ token, claims, method, path, body, timestamp, nonce, signature }) {
  if (!claims.device_key) return false;
  const deviceKey = crypto.createPublicKey({
    key: Buffer.from(claims.device_key, 'base64'), format: 'der', type: 'spki',
  });
  const field = (b) => {
    const n = Buffer.alloc(4);
    n.writeUInt32BE(b.length);
    return Buffer.concat([n, b]);
  };
  const signed = Buffer.concat([
    field(Buffer.from('sgreq.v1')),
    field(Buffer.from(token)),
    field(Buffer.from(method)),
    field(Buffer.from(path)),
    field(crypto.createHash('sha256').update(body).digest()),
    field(Buffer.from(String(timestamp))),
    field(Buffer.from(nonce, 'base64')),
  ]);
  return crypto.verify('sha256', signed, deviceKey, Buffer.from(signature, 'base64'));
}
```

Çağırmadan önce timestamp'i kontrol edin (2. adım), nonce'u yalnızca `true`
dönerse kaydedin (5. adım). `body`, request body'nin tam olarak geldiği gibi ham
byte'larıdır.

### 10.5 Önerilen policy

- **Önce gözlemleyin.** Sonuçları log'layın ve gerçek kullanıcılarınızın ne
  aldığını dashboard'da görene kadar hiçbir şeyi kısıtlamayın.
- **`ALLOW_LIMITED` yüzünden giriş ya da temel kullanımı asla engellemeyin.**
  Eski cihazlar, emulator'ler ve kendi geliştiricileriniz dahil root'lu
  telefonlar oraya meşru olarak düşer.
- **Yalnızca belirli hassas işlemleri kısıtlayın**; ödeme ya da bir key'in
  verilmesi gibi. Reddetmek yerine step-up tercih edin.
- **İstediğiniz zaman değiştirin.** Policy backend'inizde durur; uygulama
  güncellemesi olmadan gevşetip sıkılaştırabilirsiniz.

### 10.6 SDK dağıtıyorsanız

SDK'nız StringGuard kullanıyor ve müşterinizin uygulaması da kullanıyorsa:

- **Tenant'a özel bir key alias kullanın.** `deviceKeyAlias(tenantId)`'ye kendi
  tenant id'nizi verin. Aynı alias'ı paylaşan iki entegrasyon birbirinin cihaz
  key'ini ezer.
- **StringGuard package'larını shade etmeyin, yeniden adlandırmayın.** Aynı
  uygulamadaki entegrasyonlar cihaz kontrolünü paylaşır, ikincisi açılışa çok az
  maliyet ekler; shade edilmiş bir kopya bunu paylaşamaz ve runtime guard ile de
  çakışır.
- **Uygulamanız kendi domain'inizle konuşsun.** StringGuard lisansınızı
  sunucunuzda tutun ve endpoint'leri kendi API'niz üzerinden açın. Böylece
  müşterilerinizin uygulamaları seçmedikleri üçüncü bir tarafı çağırmaz ve
  lisansınız hiçbir uygulamanın içinde ship edilmez.

### 10.7 Servisi çalıştırma

Servis sizin altyapınızda çalışır; cihaz trafiği ve aşağıdaki kayıtlar oradan
çıkmaz. Java olan herhangi bir host'ta açıp çalıştırdığınız bir arşiv olarak gelir.

#### 10.7.1 Gerekenler

| | |
|---|---|
| Java | 17 veya üstü. Bulduğu runtime çok eskiyse servis bunu açıkça söyler. |
| Bir data dizini | Yazılabilir, restart'tan sonra da duran bir storage'da ve **backup'larınıza dahil**. Lisans kataloğunuzu ve uygulamanızı kullanmış cihazların listesini tutar. |
| Önünde TLS | Servis kendi ingress'inizin arkasında durmayı bekler ve ona plain HTTP konuşur. |
| Attestation root'ları | Google hardware attestation root'ları ve Apple App Attest root'u, [10.7.2](#1072-kurulum)'deki gibi indirilir. En az biri olmadan servis başlamaz. |
| Session imzalama key'leri | [10.7.2](#1072-kurulum)'deki gibi servis üretir, siz secret store'unuzda saklarsınız. Backend'iniz session token'ları bunlara karşılık gelen public key'lerle doğrular. |

#### 10.7.2 Kurulum

Arşivi indirip doğrulayın:

```bash
curl -fsSLO https://duranun.github.io/stringguard/service/stringguard-attest-service-1.0.0.tar
curl -fsSLO https://duranun.github.io/stringguard/service/stringguard-attest-service-1.0.0.tar.sha256
shasum -a 256 -c stringguard-attest-service-1.0.0.tar.sha256
tar -xf stringguard-attest-service-1.0.0.tar
cd stringguard-attest-service-1.0.0
```

Session imzalama key'lerini bir kez oluşturun ve yazılan değeri secret
store'unuza `SG_SESSION_KEYS` olarak kaydedin:

```bash
bin/stringguard-attest keys
```

Daha sonra rotate etmek için mevcut değerle çalıştırın. Yeni key o andan
itibaren imzalar, eskisi daha önce verdiği token'ları doğrulamaya devam eder:

```bash
SG_SESSION_KEYS="<mevcut değer>" bin/stringguard-attest keys --rotate
```

Ship ettiğiniz platformların attestation root'larını indirin:

```bash
# Android (jq gerekir)
curl -fsSL https://android.googleapis.com/attestation/root | jq -r '.[]' > google-roots.pem
# iOS
curl -fsSLO https://www.apple.com/certificateauthority/Apple_App_Attestation_Root_CA.pem
```

Operatör token'ını oluşturun, bunu da secret store'unuza kaydedin:

```bash
openssl rand -hex 32
```

#### 10.7.3 Ayarlar

Tüm konfigürasyon environment'tan okunur.

| Ayar | |
|---|---|
| `SG_PROFILE` | `production` yapın. Başka her değer serbest modda çalışır ve bunu söyler. |
| `SG_LICENSE_PUBLIC_KEY` | Lisansınızla birlikte verilen lisans public key'i. |
| `SG_SESSION_KEYS` | `bin/stringguard-attest keys` komutunun yazdığı değer. |
| `SG_ANDROID_ROOTS` | Google root sertifikaları dosyasının yolu; birden fazlası `;` ile ayrılır. PEM veya DER. |
| `SG_APPLE_ROOT` | Apple App Attest root sertifikasının yolu. PEM veya DER. |
| `SG_DATA_DIR` | Data dizini. Varsayılan `./data`. |
| `SG_HOST`, `SG_PORT` | Cihaz API'sinin dinlediği yer. Varsayılan tüm arayüzlerde 8787. |
| `SG_ADMIN_HOST`, `SG_ADMIN_PORT` | Operatör API'sinin dinlediği yer. Varsayılan loopback; port ayarlanana kadar hiç dinlemez. |
| `SG_ADMIN_TOKEN` | Operatör credential'ı; hem API hem dashboard kullanır. En az 32 rastgele karakter. |
| `SG_DASHBOARD` | Dashboard olmadan yalnızca API sunmak için `off`. |
| `SG_AUDIT_LOG` | Karar log'unun yolu. Verilmezse log yazılmaz. |
| `SG_SESSION_TTL_MS` | Session token'ın ne kadar geçerli kalacağı. Varsayılan 15 dakika. |
| `SG_RATE_PER_MIN` | Cihaz API'sinde çağıran başına dakikalık request sayısı. Varsayılan 120. |
| `SG_THREADS` | Request eşzamanlılığı. Varsayılan olarak host'a göre belirlenir. |
| `SG_ALLOW_APPLE_DEVELOPMENT` | Ayarlamayın. Production'da açılması reddedilir. |

Yalnızca tek bir platform için ship ediyorsanız onun root'ları yeterlidir; diğer
platformdaki cihazlar `ALLOW_LIMITED` alır.

#### 10.7.4 Canlıya almadan önce kontrol

```
bin/stringguard-attest doctor
```

Bu konfigürasyonu kontrol eder, hiçbir şey başlatmaz. Neyin eksik, neyin
güvensiz olduğunu ve her biri için ne yapılacağını söyler; konfigürasyon deploy
için uygun değilse sıfırdan farklı bir kodla çıkar. Kurulumunuzun bir parçası
olarak çalıştırın; desteğe ihtiyacınız olursa çıktısını bize gönderin, erişemediğimiz
bir kurulumu görmenin en hızlı yolu budur.

`run` servis vermeden önce aynı kontrolü yapar ve eksik bir durumda başlamak
yerine durur.

#### 10.7.5 Çalıştırma

```
bin/stringguard-attest run
```

**Tam olarak bir instance çalıştırın.** İki instance lisansları, cihazları ve
kontrolleri paylaşmaz; cihazlar rastgeleymiş gibi başarısız olmaya başlar. Aynı
data dizininde başlatılan ikinci bir instance başlamayı reddeder. Yedeklilik
için iki instance çalıştırmak yerine orchestrator'ınızın tek instance'ı yeniden
başlatmasına izin verin.

Orchestrator'ınız için iki endpoint vardır:

| | |
|---|---|
| `GET /healthz` | Process ayakta mı. |
| `GET /readyz` | Trafik gönderilmeli mi. Gönderilmemesi gerektiğinde 503 döner (örneğin data dizini yazılamaz hale geldiyse) ve kendiliğinden düzelir. |

Liveness probe'unuzu birincisine, readiness probe'unuzu ikincisine yönlendirin.
Readiness başarısız olduğunda restart işe yaramaz; instance'ı rotasyondan
çıkarmak yarar.

#### 10.7.6 Günlük işletim

Bir lisans kataloğa operatör arayüzü üzerinden girer, sonraki her müdahale de
öyle. İki yol vardır, bir dashboard ve bir API; ikisi aynı listener'ı ve aynı
credential'ı kullanır.

**Kendi portunda dinler**, siz aksini söylemedikçe loopback'te ve yalnızca
`SG_ADMIN_PORT` ile `SG_ADMIN_TOKEN` ayarlandığında. Uygulamanızın eriştiği
yolun dışında tutun: lisans oluşturur ve iptal eder.

##### Dashboard

Operatör portunda `/ui/` adresinde. `SG_ADMIN_TOKEN` ile giriş yapın; ayrıca
oluşturulacak bir hesap yoktur.

Üç sayfası var:

- **Genel bakış**: şu an ilgilenmeniz gereken bir şey var mı; son 24 saatte
  engellenen, şüpheli işaretlenen ya da tehdit raporlayan cihazlar, cihaz başına
  bir satır.
- **Lisanslar**: her lisans, cihazların eşleşmesi gereken uygulama kimliği ve
  onu kullanmış cihazlar, her birinin en son ne zaman görüldüğüyle. Buradan
  lisans ekleyin, güncelleyin, iptal edin ya da kaybolan veya değiştirilen bir
  cihazı kaldırın.
- **Güvenlik olayları**: her cihaz kontrolünde ne olduğu ve neden; varsayılan
  olarak yalnızca sorunlar. Operatör değişiklikleri altta ayrıca listelenir.

Erişmeden önce bilmeniz gereken iki şey:

- **Secure context gerekir.** Session cookie `Secure` işaretlidir; giriş HTTPS
  üzerinden ya da `localhost`'tan çalışır (en kolayı loopback portuna bir SSH
  tunnel), uzak bir adrese plain HTTP ile çalışmaz. Giriş formu sürekli geri
  geliyorsa sebebi budur.
- **Sayfalarda JavaScript yoktur**; dashboard script'ler kapalıyken de çalışır.

##### API

Kendi araçlarınız için; örneğin billing sisteminizdeki bir provisioning adımı.
Her çağrı `Authorization: Bearer $SG_ADMIN_TOKEN` taşır.

| | |
|---|---|
| `PUT /admin/licences/{id}` | Lisans ekler veya günceller: attestation'ların eşleşmesi gereken uygulama kimliği. |
| `GET /admin/licences` | Katalog; her lisansı kaç cihazın kullandığı ve iptal edilip edilmediğiyle. |
| `POST /admin/licences/{id}/revoke` | İptal eder. Herhangi bir cihazdan gelen bir sonraki çağrıda etkili olur. |
| `GET /admin/licences/{id}/devices` | Lisansı kullanmış cihazlar. |
| `DELETE /admin/licences/{id}/devices/{deviceId}` | Bir cihazı ve onun hakkında tutulan her şeyi kaldırır. Bir cihaz için silme talebini de böyle karşılarsınız. |
| `GET /admin/licences/{id}/devices/{deviceId}/keys` | Cihazın attest ettiği key'ler; bir rotasyon ya da olay incelemesi için. |

Lisans eklemek şöyle görünür:

```
curl -X PUT -H "Authorization: Bearer $SG_ADMIN_TOKEN" \
  -d '{"packageNames":["com.example.app"],
       "signingCertSha256":["<imza sertifikanızın sha256'sı>"],
       "appleAppId":"TEAMID.com.example.app"}' \
  "http://127.0.0.1:$SG_ADMIN_PORT/admin/licences/lisans-id-niz"
```

Bir lisans en az bir platformda uygulamanızı belirtmelidir.

##### Her iki yolda da

Servisin neye izin verdiğini değiştiren işlemler (lisans oluşturma veya
güncelleme, iptal, cihaz kaldırma) hangi yoldan geldikleriyle birlikte karar
log'una yazılır. Okumalar log'lanmaz.

##### Ne olduğunu görmek

**Güvenlik olayları** sayfası karar log'unu en yeniden eskiye gösterir: her
cihaz kontrolünün sonucu ve arkasındaki sebepler. Bir lisansın kendi sayfası
son sorunlarını gösterir ve aynı görünümün o lisansa daraltılmış haline
bağlanır. `SG_AUDIT_LOG` ayarlı olmalıdır; değilse sayfa boş bir tablo
göstermek yerine bunu söyler.

Debugger, hooking ve root bulguları her kontrolün sebeplerinde, cihaz ve lisans
bazında görünür. Session'ın ilerleyen bir anında olanı kaydetmek için uygulama
onu olduğu anda raporlayabilir:

```
POST /attest/report { "sessionToken": "...", "categories": ["HOOKING_FRAMEWORK"] }
```

Uygulamanın zaten elinde olan session token'ı kullanır ve log'da
`CLIENT_REPORT` olarak görünür. **Bir rapor hiçbir kararı değiştirmez**;
kullanıcılarınızdaki eğilimleri görebilmeniz içindir. Hooking framework
raporlayan tek bir cihaz pek bir şey ifade etmez, tek bir release'te dört yüz
cihaz ise harekete geçmeniz gereken bir durumdur.

Bu servis olmadan README'nin runtime guard bölümündeki `Detections` callback'ini
kullanın.

#### 10.7.7 Size kalanlar

| | |
|---|---|
| TLS termination ve gerçek client adresinin iletilmesi | Rate limiting proxy başına değil çağıran başına saysın diye. |
| Session imzalama private key'lerinin saklanması | Policy'niz gerektiriyorsa environment yerine bir KMS veya HSM. |
| Root sertifika dosyalarını güncel tutmak | Google ve Apple zaman zaman yenilerini yayınlar; onlar olmadan gerçek cihazlar reddedilir. |
| Data dizininin backup'ı | Kaybederseniz kataloğunuzu ve cihaz geçmişinizi kaybedersiniz. |
| Kapasite | Tek instance'ı trafiğinize göre boyutlandırın: `SG_THREADS` ve bellek limiti, daha fazla replica değil. |
| Audit log saklama süresi | Kendi policy'nize göre. |

Baştan edinmeye değer bir alışkanlık: **session key'lerini değiştirirken
silmeyin, ekleyin.** Yeni key'i current olarak ekleyin, eskisini doğrulama için
tutun ve token'larının süresi dolunca kaldırın. Doğrudan değiştirmek, dolaşımdaki
tüm session token'ları geçersiz kılar.

Servisin neyi işlediği, sakladığı ve hiç dokunmadığı (DPA'nız için girdi) için
bkz. [DATA_HANDLING.md](DATA_HANDLING.md).

## 11. Mevcut bir projeye geçiş

1. **Plugin'i [bölüm 3](#3-stringguardı-ekleme)'teki gibi ekleyin** ve
   `enabled = true` ile bir kez build alın. Obfuscate-all varsayılan olduğu için
   ilk build'de, kod değişikliği olmadan uygun her şey obfuscate edilir.

2. **Bir release build alın, test suite'inizi ve smoke test'lerinizi
   çalıştırın.** `const val` ve annotation argümanları zaten korunur.
   Uygulamanız runtime'da bir string literal'i alışılmadık bir yerde değerine
   bakarak okuyorsa (bir literal üzerinden reflection, literal'e göre çalışan
   elle yazılmış bir resource lookup gibi), onu iki yoldan biriyle hariç tutun:
   - o kapsamı `@DontObfuscate` ile işaretleyin, ya da
   - bir `keepMatching` veya `keepPackage` kuralı ekleyin.

3. **`minLength`'i yükseltin**, örneğin `4`'e; önemsiz literal'ler atlanır,
   çıktı küçülür.

4. **[Bölüm 7](#7-binary-doğrulama)'deki plain text sızıntı kontrolünü** CI'a
   ekleyin; plain text olarak eklenen yeni bir secret otomatik yakalanır.

5. **İsterseniz opt-in modeline geçin.** `obfuscateAll = false` yapıp hassas
   birkaç kapsamı `@Obfuscate` ile işaretleyin.

Şunlar için bir şey yapmanız gerekmez, oldukları gibi güvenlidir: Retrofit ve
Ktor endpoint annotation'ları, `@SerialName` ile tanımlanmış `@Serializable`
alan isimleri, `const val` olarak tanımlanmış log tag'leri ve annotation
argümanı kullanan dependency injection qualifier'ları.

## 12. Sorun giderme

**`Incompatible Gradle JVM version … supports Java 1.8 and 24` (Android Studio).**
Android Studio'nun bundled JBR'ı JDK 25 olabilir, Gradle 8.14 bunu reddeder.
Gradle JDK'yı 17–24 aralığında bir sürüme ayarlayın:

- *Settings → Build, Execution, Deployment → Build Tools → Gradle → Gradle JDK*, ardından 17–21 aralığında bir JDK seçin veya ekleyin, **ya da**
- `gradle.properties` içinde `org.gradle.java.home=/path/to/jdk21` ayarlayın (ve/veya IDE'nin `#GRADLE_LOCAL_JAVA_HOME` seçeneğinin okuduğu `.gradle/config.properties` içinde `java.home=…`),

sonra **Sync Project with Gradle Files** çalıştırın. *"Apply compatible Gradle
JDK configuration"* seçeneğine neyi seçtiğine bakmadan tıklamayın; uygun
olmayan eski bir JDK seçebilir.

**`Plugin [id: 'com.stringguard'] was not found`.** Lisansınızda belirtilen
repository `pluginManagement.repositories` içinde yok
([bölüm 2.1](#21-repository-tanımı)).

**`Could not find com.stringguard:runtime` / `:annotations`.** Repository
`dependencyResolutionManagement.repositories` içinde yok. Yalnızca
`pluginManagement`'ta değil, iki blokta da olmalı.

**Kotlin sürümünü yükselttikten sonra compiler plugin sürüm veya ABI hatası.**
Compiler plugin'i belirli bir Kotlin sürümüne, 2.2.0'a göre build edilir.
Projenizin Kotlin sürümünü eşit tutun ya da sizinkine göre derlenmiş bir
StringGuard build'i kullanın.

**Şifrelenmesini beklediğiniz bir literal hâlâ plain text.** Sırasıyla kontrol
edin: `const val` mı; annotation argümanı mı; `minLength`'ten kısa mı;
`@DontObfuscate` veya `@KeepPlain` kapsamında mı; bir `keepRules` pattern'ine
uyuyor mu; ya da `enabledForDebug = false` iken bir debug build mi.

**Bir string obfuscate edilmiş ama runtime'da okunabilir kalması gerekiyor
(reflection).** O kapsamı `@DontObfuscate` ya da bir `keepRules` kaydıyla
hariç tutun.

## 13. SSS

**Bir şeyi annotate etmem gerekiyor mu?** Hayır. Obfuscate-all varsayılandır.
`@DontObfuscate` ve `@KeepPlain` hariç tutmak içindir; `@Obfuscate` yalnızca
opsiyonel opt-in modunda kullanılır.

**R8 veya ProGuard olmadan çalışır mı?** Evet. StringGuard R8'den bağımsızdır ve
Kotlin compiler'ının içinde çalışır. İkisi birbirini tamamlar ve birlikte
kullanılabilir: R8 isimleri değiştirip küçültür, StringGuard string
içeriklerini şifreler.

**Uygulamayı yavaşlatır mı?** Decrypt literal başına yapılır; küçük ve senkron
bir işlemdir. Tipik secret ve konfigürasyon string'leri için maliyeti
önemsizdir. Bir fark ölçerseniz çok büyük ya da sık çalışan döngülerdeki
literal'leri `minLength` veya `keepRules` ile hariç tutun.

**Bu gerçek bir güvenlik mi?** StringGuard statik analizin maliyetini yükseltir;
basit `strings` incelemesini, decompile'ı ve otomatik scanner'ları boşa
çıkarır. Kriptografik gizlilik değildir: decrypt cihazda yapıldığı için anahtar
da orada elde edilebilir. Asla sızmaması gereken bir değerin yeri sunucudur.
Güvenlik modelinin tamamı README'de.

**Multiplatform'da iOS ve Wasm gerçekten kapsanıyor mu?** Evet. Runtime
`commonMain`'e eklenir ve aynı şifreli payload JVM, Kotlin/Native (iOS), JS ve
Wasm'da birebir aynı çözülür. Native, JS ve Wasm target'larının obfuscation'ı
için ayrı bir hak gerekmez: tek lisans tüm target'ları kapsar.

**Şifrelemeyi ChaCha20'ye çevirebilir miyim?** Evet.
`stringGuard { }` bloğunda `algorithm = StringGuardAlgorithm.CHACHA20_POLY1305`
ayarlayın. Runtime hem AES-GCM hem ChaCha20-Poly1305 payload'larını her
target'ta çözer, istediğiniz zaman geçebilirsiniz. Varsayılan AES-GCM'dir.

**Anahtarı uygulama imzasına bağlamak mümkün mü?** Evet, `tamperBinding` ile
(yalnızca Android). Kurulumu, Play App Signing notunu ve sınırlarını
[README](../README.tr.md#tamper-binding-android) anlatıyor.
