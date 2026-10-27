# StringGuard Entegrasyon Kılavuzu

[English](INTEGRATION.md) · **Türkçe**

Bu kılavuz, mevcut bir Kotlin projesine StringGuard'ın baştan sona
entegrasyonunu anlatır: araç zinciri gereksinimleri,
kurulum, platform bazlı modül yapılandırması, ilk build, obfuscation kapsamının
denetimi, CI ve sorun giderme. Entegrasyonu yapan geliştirici için yazılmıştır.
Kısa özet için [README](../README.tr.md) dosyasına bakın.

**İçindekiler**
1. [Gereksinimler](#1-gereksinimler)
2. [Kurulum](#2-kurulum)
   - [Repository tanımları](#21-repository-tanımları)
3. [StringGuard'ın uygulanması](#3-stringguardın-uygulanması)
   - [Düz Android](#31-düz-android-klasik-kmp-olmayan)
   - [JVM / Desktop](#32-jvm--desktop)
   - [Kotlin Multiplatform / Compose Multiplatform](#33-kotlin-multiplatform--compose-multiplatform)
   - [iOS: framework'ün linklenmesi](#34-ios-frameworkün-linklenmesi)
   - [Flutter](#35-flutter)
   - [Doğrudan Swift ile yazılmış uygulamalar](#36-doğrudan-swift-ile-yazılmış-uygulamalar)
4. [İlk build](#4-i̇lk-build)
5. [Yapılandırma](#5-yapılandırma)
6. [Debug, release ve CI](#6-debug-release-ve-ci)
7. [Binary doğrulaması](#7-binary-doğrulaması)
8. [Lisanslama](#8-lisanslama)
   - [Anahtarın verilmesi](#81-anahtarın-verilmesi)
   - [Geçerli anahtar olmadığında](#82-geçerli-anahtar-olmadığında-ne-olur)
   - [Yenileme ve değiştirme](#83-yenileme-ve-değiştirme)
9. [App Store'a gönderim](#9-app-storea-gönderim)
10. [Mevcut kod tabanının taşınması](#10-mevcut-kod-tabanının-taşınması)
11. [Sorun giderme](#11-sorun-giderme)
12. [SSS](#12-sss)

## 1. Gereksinimler

| Araç | Sürüm |
|---|---|
| JDK | 17–24 (21 önerilir). **25/26 olmaz**: Gradle 8.14, Java > 24 sürümlerini kabul etmez. |
| Gradle | 8.x (8.14 ile test edildi) |
| Kotlin | **2.2.0**; eklentinin compiler build'iyle birebir aynı olmalıdır |
| AGP | 8.x (Android) |

Compiler plugin, Kotlin sürümüne bağlıdır. Projeniz Kotlin 2.2.0 kullanmıyorsa
entegrasyona başlamadan önce sürümü hizalayın veya kendi Kotlin sürümünüze karşı
derlenmiş bir StringGuard build'ini bekleyin.

Varsayılan davranış obfuscate-all'dır. Literal'ler tek tek kapsama alınmaz,
aksine kapsam dışına çıkarılır. Derleme zamanında sabit kalmak zorunda olan
literal'lere, yani `const val` initializer'larına ve annotation argümanlarına
hiçbir koşulda dokunulmaz. Bu nedenle eklentinin proje genelinde açılması, bu
değerlere dayanan kodu bozmaz.

## 2. Kurulum

### 2.1 Repository tanımları

1. Projenizin `settings.gradle.kts` dosyasında depoyu **her iki** bloktan da
   erişilebilir yapın:

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

   Kimlik bilgisi gerekmez.

2. Projeyi sync edin. Gradle, eklentinin kendisini `pluginManagement` üzerinden;
   eklentinin sizin adınıza eklediği `annotations` ve `runtime` kütüphanelerini
   ise `dependencyResolutionManagement` üzerinden çözer. Satırın iki blokta da
   bulunması gerekir; biri eksik kalırsa
   [11. bölümdeki](#11-sorun-giderme) çözümleme hatalarından biriyle
   karşılaşırsınız.

## 3. StringGuard'ın uygulanması

Aşağıdan kendi projenizi bulun. 3.1–3.4 bölümleri Kotlin projelerini kapsar ve
Gradle eklentisini kullanır; 3.5 ve 3.6 bölümleri ise farklı yapılandırılan
Flutter ve düz Swift'i kapsar.

| Projeniz | Bölüm | Metinler nasıl korunur |
|---|---|---|
| Android uygulaması veya kütüphanesi, multiplatform değil | [3.1](#31-düz-android-klasik-kmp-olmayan) | Gradle eklentisi, modüldeki her metin |
| JVM veya Desktop | [3.2](#32-jvm--desktop) | Gradle eklentisi, modüldeki her metin |
| Kotlin Multiplatform veya Compose Multiplatform (Android, iOS, JVM, JS, Wasm) | [3.3](#33-kotlin-multiplatform--compose-multiplatform) | Gradle eklentisi, her metin, her hedef |
| Kotlin framework'ü tüketen bir iOS uygulaması | [3.3](#33-kotlin-multiplatform--compose-multiplatform) + [3.4](#34-ios-frameworkün-linklenmesi) | 3.3 gibi; 3.4 isteğe bağlı guard ve attestation içindir |
| Flutter veya Dart | [3.5](#35-flutter) | Kod üreteci, bir secrets dosyasından |
| Doğrudan Swift ile yazılmış, Kotlin içermeyen | [3.6](#36-doğrudan-swift-ile-yazılmış-uygulamalar) | Xcode derleme adımı, kaynaklarınızı yeniden yazar |

Kotlin bölümleri için: StringGuard'ı, modülün hâlihazırda kullandığı Kotlin
eklentisinin yanına uygulayın. Sıra önemli değildir; StringGuard, Kotlin
eklentisi hangi anda uygulanırsa kendisini ona bağlar. `annotations` ve
`runtime` bağımlılıklarını elle eklemeyin; eklenti bunları doğru configuration
veya source set'e kendisi ekler.

### 3.1 Düz Android (klasik, KMP olmayan)

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
        // Önerilen. StringGuard iki durumda da çalışır; aşağıdaki nota bakın.
        release { isMinifyEnabled = true }
    }
}

stringGuard {
    enabled = true
    minLength = 4
    licenseKey = providers.environmentVariable("STRINGGUARD_LICENSE").getOrElse("")
}
```

Bağımlılıklar `implementation` configuration'ına eklenir.

**Release derlemelerinde küçültmeyi açık bırakın.** StringGuard R8'e ihtiyaç
duymaz ve onsuz da çalışır; ancak R8, çalışma zamanı korumasının iç
sembollerini — dedektör giriş noktalarını ve çözme giriş noktasını — yeniden
adlandırır ve StringGuard bunu mümkün kılan kuralları birlikte taşır.
Küçültme kapalı derlenen bir release, bu adları APK içinde okunabilir bırakır;
bu da onu decompile eden birine kontrollerin tam olarak nerede olduğunu söyler.
Bu sertleştirme zaten sizde; mesele yalnızca onu kapatmamak.

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

Bağımlılıklar `implementation` configuration'ına eklenir.

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

`annotations` ve `runtime` bağımlılıkları `commonMain` source set'ine eklenir;
böylece Apple ve Wasm hedefleri dahil olmak üzere her hedef aynı yapılandırmayla
kapsanır. Hedef bazlı ek bir wiring gerekmez. Her hedef aynı biçimde obfuscate
edilir: tek bir lisans ve tek bir özellik kümesi olduğundan Apple, JS ve Wasm
hedefleri JVM ve Android'den farklı ele alınmaz.

**JS ve Wasm** için yukarıdakinin ötesinde bir şey gerekmez. Runtime saf
Kotlin'dir; bu yüzden korunan bir değer `js(IR)` ve `wasmJs` üzerinde JVM'de
olduğu gibi birebir aynı şekilde çözülür. Tarayıcı ve Node hedefleri aynı
davranır.

**Compose Multiplatform** bu bölümün kapsamındadır: o da bir Multiplatform
projesidir ve eklenti Compose'a özel bir davranış uygulamaz. Composable
fonksiyonların içindeki metinler de diğerleri gibi gizlenir.

### 3.4 iOS: framework'ün linklenmesi

Bir iOS hedefinde string obfuscation için 3.3 bölümünden fazlası gerekmez. Bu
bölüm, Swift tarafına bir Kotlin framework'ü üzerinden ulaşan isteğe bağlı
çalışma zamanı koruması ve attestation istemcisi hakkındadır.

**Mevcut bir multiplatform uygulamaya eklemeden önce bunu okuyun.**
Kotlin/Native ürettiği her framework'e kendi runtime'ını gömer; bu yüzden bir
iOS uygulaması bunlardan ikisini birlikte linkleyemez. `StringGuard.framework`
tam olarak bu nedenle çalışma zamanı korumasını ve attestation istemcisini tek
bir framework içinde taşır.

`StringGuard.xcframework`, lisansınızın bir parçası olarak dağıtılır ve nereden
edineceğinizi lisansınız bildirir; hem cihaz hem simülatör dilimlerini, hata
ayıklama sembolleriyle birlikte taşır.

Uygulamanızın **kendi** paylaşılan Kotlin framework'ü yoksa onu olağan yolla
linkleyin: CocoaPods (`vendored_frameworks`), Swift Package Manager
(`.binaryTarget`) veya elle gömme — üçü de çalışır.

Uygulamanız **kendi paylaşılan Kotlin framework'ünü zaten dağıtıyorsa**,
`StringGuard.framework`'ü onun yanına linklemeyin. Kütüphaneleri kendi
framework'ünüze ekleyin:

```kotlin
// mevcut shared modülünüz
kotlin {
    sourceSets.commonMain.dependencies {
        api("com.stringguard:guard:<sürüm>")        // api, implementation DEĞİL
        api("com.stringguard:attest-core:<sürüm>")
    }
    listOf(iosArm64(), iosSimulatorArm64(), iosX64()).forEach {
        it.binaries.framework {
            baseName = "Shared"
            export("com.stringguard:guard:<sürüm>")
            export("com.stringguard:attest-core:<sürüm>")
        }
    }
}
```

`implementation` değil `api` olmalı: yalnızca bir `api` bağımlılığı export
edilebilir ve export olmadan semboller üretilen header'lara hiç ulaşmaz.

**Doğrudan Swift ile** yazılmış, hiç Kotlin içermeyen bir uygulama için bunun
yerine [3.6 bölümüne](#36-doğrudan-swift-ile-yazılmış-uygulamalar) bakın. O yol
farklı yapılandırılır ve kendi varsayılanları ile hariç tutma sözdizimi vardır.

### 3.5 Flutter

Flutter, Kotlin ve Swift yollarından **farklı bir model** kullanır ve bu fark
önemlidir: kaynağınızda zaten yazılı olan metinleri gizlemez. Bunun yerine açık
metin, kaynak ağacınızın dışındaki bir secrets dosyasında durur; bir kod üreteci
onu derleme anında okur ve gizlenmiş veriyi üretir. Kaynağınız sırrı hiç
içermez, secrets dosyası ise hiç commit'lenmez.

Üç paket, her birinin ayrı işi var:

| Paket | Rolü | Nereye yazılır |
|---|---|---|
| `stringguard` | Annotation'lar ve değerleri açan runtime | `dependencies` |
| `stringguard_generator` | Secrets dosyasını okuyup veriyi üreten `build_runner` üreteci | `dev_dependencies` |
| `stringguard_flutter` | Çalışma zamanı korumasını ve attestation'ı Dart'a açan plugin. **String obfuscation yapmaz.** İsteğe bağlı. | yalnızca bunları istiyorsanız `dependencies` |

`pubspec.yaml` dosyanızda (paketleri nereden edineceğinizi lisansınız bildirir;
pub.dev üzerinde değillerdir):

```yaml
dependencies:
  stringguard: <lisansınızın bildirdiği gibi>

dev_dependencies:
  stringguard_generator: <lisansınızın bildirdiği gibi>
  build_runner: ^2.4.0
```

Sırları tanımlayın. `secrets.env` dosyasını sürüm kontrolünün dışında tutun:

```dart
import 'package:stringguard/annotations.dart';
import 'package:stringguard/stringguard.dart';

part 'config.g.dart';

@StringGuardEnv('secrets.env')
class Config {
  @Secret()                       // alan adıyla aranır
  static final String apiKey = _$Config.apiKey;

  @Secret('API_BASE_URL')         // ya da açık bir anahtarla
  static final String baseUrl = _$Config.baseUrl;
}
```

Ardından üretin:

```bash
dart run build_runner build      # geliştirirken `watch`
```

Erişimci her zaman `_$<SınıfAdı>.<alanAdı>` biçimindedir ve üretilen part
dosyasında tanımlanır. Üreteci ilk derlemeden önce çalıştırın, aksi hâlde part
dosyası henüz var olmaz. Builder yapılandırması gerekmez: üreteç, kendisine
bağımlı olan her pakete kendiliğinden uygulanır.

Çalışma zamanı koruması veya attestation için `stringguard_flutter`'ı
eklerseniz, kendi iOS privacy manifest'ini taşıdığını unutmayın; bkz.
[9. bölüm](#9-app-storea-gönderim).

### 3.6 Doğrudan Swift ile yazılmış uygulamalar

Hiç Kotlin içermeyen bir uygulama için, Xcode derlemesi sırasında Swift
kaynaklarınızı yeniden yazan ayrı bir entegrasyon vardır; böylece sıradan string
literal'leri yazmaya devam edersiniz.

Tamamı [swift/README.md](../swift/README.md) içinde olan üç adım:

1. Swift paketini ekleyin ve `StringGuardRuntime` ürününü linkleyin.
2. Compile Sources'tan **önce** bir **Run Script** derleme fazı ekleyin; bu faz
   kaynaklarınızı derleme dizinine yeniden yazar ve bir sanal dosya sistemi
   (VFS) overlay'i üretir.
3. Build Settings içinde derleyiciyi bu overlay'e yönlendirin — **Other Swift
   Flags**: `-vfsoverlay $(DERIVED_FILE_DIR)/stringguard/overlay.yaml`.

Kendi kaynak dosyalarınız hiç değiştirilmez; derleyici yeniden yazılmış
kopyaları okur.

**Bu yol Kotlin'den farklı yapılandırılır; karma bir kod tabanında tek bir
politikanın her ikisini kapsadığını varsaymayın.** Yapmanız gerekeni değiştiren
farklar:

| | Kotlin | Swift |
|---|---|---|
| En küçük metin uzunluğu | `minLength`, varsayılan 1 | `--min-length` bayrağı, varsayılan **4** |
| Hariç tutma | `@DontObfuscate` / `@KeepPlain`, `keepRules` | `// stringguard:keep` yorumu veya hariç tutulan bir yol |
| İnterpolasyonlu literal'ler | gizlenir | **yeniden yazılmaz** |
| Çok satırlı ve raw literal'ler | gizlenir | **yeniden yazılmaz** |

İnterpolasyonlu literal'lerin kapsam dışı olması, planlamanız gereken fark:
interpolasyonla kurulan bir sır okunabilir kalır. Bu yolda sırları bütün birer
literal olarak tutun. Platform alt sınırları ve hiç yeniden yazılmayanların tam
listesi [swift/README.md](../swift/README.md) içindedir.

## 4. İlk build

Projeyi her zamanki gibi derleyin:

```bash
# Android
./gradlew assembleRelease

# JVM / KMP
./gradlew build
```

Kaynak kodunuz değişmez. Sıradan string literal'leri yazmaya devam edersiniz:

```kotlin
object Config {
    val apiBaseUrl = "https://api.example.com/v2"   // release binary'de obfuscate edilir
    val analyticsKey = "ak_prod_1a2b3c4d5e"         // obfuscate edilir
}
```

Çalışma anındaki değerler yazdığınız değerlerle birebir aynıdır; binary içinde
ise bulunmazlar. Bunu [7. bölümdeki](#7-binary-doğrulaması) kontrollerle
doğrulayın.

## 5. Yapılandırma

`stringGuard { }` bloğu aşağıdaki seçenekleri kabul eder. Seçenek
belirtilmediğinde varsayılan değer geçerlidir.

| Seçenek | Varsayılan | Etkisi |
|---|---|---|
| `enabled` | `true` | Ana anahtar. `false` olduğunda compiler plugin hiçbir iş yapmaz. |
| `obfuscateAll` | `true` | Proje genelinde uygun her literal'i obfuscate eder. `false` olduğunda yalnızca `@Obfuscate` kapsamları obfuscate edilir. |
| `minLength` | `1` | Bu karakter sayısından kısa literal'ler düz bırakılır. |
| `enabledForDebug` | `false` | Debug build türlerini de obfuscate eder. Test compilation'ları her iki durumda da atlanır. |
| `algorithm` | `StringGuardAlgorithm.AES_GCM` | AEAD primitifi. Alternatifi `StringGuardAlgorithm.CHACHA20_POLY1305` değeridir. |
| `licenseKey` | `""` | **Zorunlu.** Offline lisans token'ı veya modül dizinine göre göreli bir lisans dosyası yolu. Eksik, bozuk veya süresi dolmuş değer build'i kırar. Bkz. [8. bölüm](#8-lisanslama). |
| `keepRules { }` | kural yok | Aşağıda açıklanan package ve regex tabanlı hariç tutma kuralları. |

### `@DontObfuscate` ile kapsam hariç tutma

İçindeki tüm literal'lerin okunabilir kalması için bir dosyayı, class'ı,
object'i, fonksiyonu, property'yi veya constructor'ı işaretleyin:

```kotlin
import com.stringguard.annotations.DontObfuscate

@DontObfuscate                        // dosya, class, object, function, property veya constructor
object ReflectionKeys {
    const val COLUMN = "user_id"      // (const val zaten atlanırdı)
    val lookupName = "com.example.Thing"   // Class.forName tarafından okunur, okunabilir kalmalı
}
```

`@KeepPlain` aynı etkiyi üretir ve aynı kod yolu tarafından dikkate alınır.
Yalnızca annotation hedefleri bakımından farklıdır: `@DontObfuscate` bir dosyanın
tamamına uygulanabilirken `@KeepPlain` uygulanamaz.

### `keepRules` ile kalıp tabanlı hariç tutma

```kotlin
stringGuard {
    minLength = 4
    keepRules {
        keepPackage("com.example.resources.*")   // paket ve tüm alt paketleri düz kalır
        keepMatching("^https://cdn\\.")           // regex'e uyan literal'ler düz kalır
        keepMatching("^android\\.intent\\.")      // ör. sistem intent action string'leri
    }
}
```

`keepPackage("a.b.*")` hem `a.b` paketini hem de altındaki tüm alt paketleri
korur. `keepMatching(regex)` içeriği eşleşen her literal'i korur; eşleşme
`containsMatchIn` ile değerlendirilir.

### Hiçbir zaman obfuscate edilmeyen literal'ler

Eklenti aşağıdakileri her zaman derleme zamanı sabiti olarak bırakır, çünkü
obfuscate edilmeleri hâlinde kod derlenmez. Bunlar için kural tanımlamanız
gerekmez.

- `const val` initializer'ları
- Annotation argümanları (`@SerialName("…")`, `@GET("…")`, `@Deprecated("…")`, …)
- Annotation class default değerleri

Retrofit, kotlinx.serialization ve Room'un obfuscate-all açıkken çalışmaya devam
etmesinin nedeni budur.

### Seçerek etkinleştirme (opt-in) modu

Yalnızca açıkça işaretlenen kapsamların obfuscate edilmesi için proje genelindeki
obfuscation'ı kapatın:

```kotlin
stringGuard { obfuscateAll = false }
```

Ardından kapsama almak istediğiniz yerleri işaretleyin:

```kotlin
import com.stringguard.annotations.Obfuscate

@Obfuscate
class PaymentClient {
    private val merchantSecret = "msk_live_..."   // obfuscate edilir
}
// @Obfuscate kapsamları dışındaki her şey düz kalır
```

## 6. Debug, release ve CI

Hangi compilation'ların obfuscate edileceğini üç kural belirler:

- Test compilation'ları her zaman atlanır ve hiçbir koşulda obfuscate edilmez.
  Bunlar Kotlin ve AGP'nin `test`, `testFixtures` olarak ya da `Test` ile biten
  adlarla isimlendirdiği compilation'lardır: `jvmTest`, `debugUnitTest`,
  `freeDebugAndroidTest` gibi.
- Debug build türleri, `enabledForDebug = true` verilmedikçe düz bırakılır. Bu,
  `debug` ile birlikte flavor'lı tüm debug varyantlarını kapsar (`prodDebug`,
  `freeDebug`).
- Geri kalan her şey obfuscate edilir; `staging` veya `qa` gibi özel build
  türleri de dahil. `enabledForDebug` bunları düz bırakamaz; bir keep kuralı
  kullanın.

Sınıflandırma tam ad ve sonek eşleşmesiyle yapılır, alt dize ile değil; bu
yüzden adı yalnızca "test" veya "debug" içeren bir product flavor — `latest`,
`contest`, `fastest` — test ya da debug compilation'ı sanılmaz.

```kotlin
stringGuard {
    enabledForDebug = false   // dev build'leri hızlı, loglar greplenebilir kalsın
}
```

Debug build'lerinin varsayılan olarak düz kalmasının nedeni, obfuscation'ın
literal başına küçük bir maliyet getirmesi ve geliştirme sırasında `strings`
çıktısı ile logcat'in okunmasını zorlaştırmasıdır. Korunması gereken, dağıtıma
çıkan yapılandırmadır.

CI'ın lisansa ve sabitlenmiş bir JDK'ya ihtiyacı var; başka bir şeye yok. Tipik
bir release hattı şu komutları çalıştırır:

```bash
export STRINGGUARD_LICENSE="<CI secret deponuzdan enjekte edilir>"
./gradlew clean assembleRelease      # veya :app:bundleRelease
```

Token'ı CI secret'ı olarak enjekte edin, asla commit'lemeyin. Eksik olduğunda
build kırıldığı için, onu eklemeyi atlayan bir hat sessizce korumasız metin
dağıtamaz.

CI, JDK 25 veya 26 içeren bir container image kullanıyorsa 17–24 aralığında bir
JDK sabitleyin; bkz. [11. bölüm](#11-sorun-giderme). Bir sonraki bölümdeki düz
metin sızıntısı kontrolünü build geçidi olarak ekleyin.

## 7. Binary doğrulaması

Korunan literal'ler derlenmiş çıktıda görünmez. Her StringGuard sürümü, bunun
desteklenen her backend'de gösterilmesi şartına bağlıdır; dolayısıyla bu,
sizin tespit etmeniz gereken bir şey değil, sürümün bir özelliğidir.

Kendi derlemenizde görmek isterseniz en hızlı yer JVM çıktısıdır:

```bash
./gradlew build
javap -c -p -classpath build/libs/<sizin>.jar com.example.Config
# sır literal'leri yok
```

Derlenmiş bir ikili üzerinde düz metin araması her platformda güvenilir bir
kontrol değildir; sonuç değil, sonuçsuz sayılmalıdır.

## 8. Lisanslama

Lisans **zorunludur**. StringGuard'ın tek bir özellik kümesi ve tek bir anahtar
türü vardır: geçerli bir lisans her şeyi, her hedefte etkinleştirir; ücretsiz,
deneme veya ara bir kademe yoktur.

Doğrulama offline'dır. Token, eklentiye gömülü bir public key ile yapılandırma
anında ve ağ çağrısı olmadan denetlenir; bu yüzden CI'da ve ağdan yalıtılmış
derlemelerde de çalışır.

### 8.1 Anahtarın verilmesi

```kotlin
stringGuard {
    licenseKey = "<token>"      // veya lisans dosyasının yolu (modüle göre göreli)
}
```

Token gizlidir; sürüm kontrolünden uzak tutun. Alışılmış düzen onu ortamdan
okumaktır, bu aynı zamanda Gradle configuration cache girdisinin dışında
kalmasını sağlar:

```kotlin
stringGuard {
    licenseKey = providers.environmentVariable("STRINGGUARD_LICENSE")
        .orElse(providers.gradleProperty("stringguard.license"))
        .getOrElse("")
}
```

CI'da secret olarak enjekte edin. Dosya yolu da çalışır; bu, bir secret
yöneticisinin mount ettiği anahtara uygundur:

```kotlin
stringGuard { licenseKey = "config/stringguard.license" }
```

### 8.2 Geçerli anahtar olmadığında ne olur

**Build kırılır.** Yapılandırma, istediğinizden daha azını koruyan bir çıktı
üretmek yerine açıklayıcı bir hatayla durur:

| Durum | Sonuç |
|---|---|
| `licenseKey` verilmemiş, boş ya da boş bir dosya | `StringGuard requires a license key. ...` |
| StringGuard token'ı değil, kesilmiş ya da başka bir anahtarla imzalanmış | `StringGuard license is not valid: <reason>. ...` |
| Kullanımdan kaldırılmış v1 biçiminde kesilmiş | `... license format v1 is no longer supported; reissue the key ...` |
| Son kullanma gününü geçmiş | `StringGuard license '<id>' expired on <date>. ...` |

Bunu zorunlu olmaktan çıkaran bir anahtar yoktur. Alternatifi — devam edip daha
azını gizlemek — yeşil bir derlemeden okunabilir sırlar taşıyan bir çıktı
üretir; bir metin gizleyicinin asla üretmemesi gereken sonuç tam olarak budur.

Gerçekten gizlenmemesi gereken bir modül devre dışı bırakılır ve o zaman
anahtara ihtiyaç duymaz:

```kotlin
stringGuard { enabled = false }
```

Başarılı bir derleme lisansı bir kez, `lifecycle` seviyesinde loglar:

```
StringGuard: license verified (holder=Acme Inc, expires=2027-04-01)
```

Gradle **configuration cache isabetinde** proje yeniden yapılandırılmadığı için
lisans yeniden doğrulanmaz ve bu satır basılmaz. İki önbelleklenmiş derleme
arasında süresi dolan bir lisans, bir sonraki soğuk yapılandırmada yakalanır.

### 8.3 Yenileme ve değiştirme

Lisans belirli bir süre için kesilir; yenilemek, build'inizin okuduğu token'ı
değiştirmek anlamına gelir. Token'ı [8.1](#81-anahtarın-verilmesi) bölümündeki
gibi ortamdan veya bir dosyadan veriyorsanız yenileme, kod değişikliği ve yeni
sürüm gerektirmeyen bir secret güncellemesidir.

Yenilemeyi payla planlayın. Doğrulama zorunlu olduğu için süresi dolan bir
anahtar derlemeleri durdurur — bir olay anındaki hotfix derlemesi dahil. Lisansa
adlı bir sahip ve son kullanma tarihinden haftalar önce uyaran bir alarm
tanımlayın, yenileme yolunu nöbetçi mühendisin bulabileceği bir yerde tutun.

Lisans, **son kullanma günü dâhil** geçerlidir ve tarih hem lisansı kesen
tarafta hem derlemede **UTC** olarak değerlendirilir. Yani 2027-04-01 tarihli
bir anahtar, agent'larınız nerede koşarsa koşsun 2027-04-01'de hâlâ derler ve
2027-04-02'de durur. Yine de pay bırakın; yenilemeyi sınıra denk getirmeyin.

Yayımlanmış uygulamalar etkilenmez: daha önce dağıttığınız çalışma zamanı
bileşenleri son kullanıcılarınız için lisanslı kalır. Yalnızca yeni derleme
üretme yeteneğiniz durur.

Yeni bir ekip, farklı bir build sistemi ya da sızıntı sonrası değişim için
anahtar gerekiyorsa olağan ticari temas noktanızdan talep edin. Sızan bir
token'ı diğer tüm kimlik bilgileri gibi değerlendirin: değiştirilmesini sağlayın
ve onu tutan her hatta secret'ı döndürün.

## 9. App Store'a gönderim

StringGuard'ı bir iOS uygulamasına gömmek iki App Store yükümlülüğü doğurur.
İkisini de gönderimi yapan geliştirici cevaplar ve ikisi de varsayılanla değil
olguyla cevaplanır.

### Şifreleme ihracat uyumu

StringGuard korunan literal'leri AES-GCM ile çözer. Apple hedeflerinde bu çözme
işini CryptoKit veya başka bir işletim sistemi framework'ü değil, StringGuard'ın
kendi implementasyonu yapar.

Apple'ın rehberi işletim sistemine gömülü şifrelemeyi, örneğin `URLSession`
üzerinden HTTPS'i, ihracat dokümantasyonu gereksiniminden genelde muaf sayar ve
bu muafiyeti uygulamanın içinde taşınan bir implementasyona genişletmez. Bu
nedenle App Store Connect'in şifreleme sorusuna verdiğiniz cevap ve
`Info.plist` içindeki `ITSAppUsesNonExemptEncryption` değeri, StringGuard
eklenmeden önceki halinden farklı olabilir. Muaf olmayan bir cevap, yıl sonu
self-classification raporu gerektirebilir.

Bu doküman sizin uygulamanız için hangi cevabın doğru olduğunu söylemez; bu
bulunduğunuz yargı alanına ve diğer bağımlılıklarınıza bağlıdır. Söylediği şey,
cevaplamak için ihtiyaç duyduğunuz olgudur: şifreleme işletim sisteminde değil,
uygulamanın içindedir. Bakınız
[Complying with Encryption Export Regulations](https://developer.apple.com/documentation/security/complying-with-encryption-export-regulations).

### Privacy manifest

Gradle plugin'i, derleyici eklentisi ve obfuscation runtime'ı required-reason
API kullanmaz; dolayısıyla tek başına string obfuscation privacy manifest'inize
bir şey eklemez.

**Çalışma zamanı koruması farklıdır.** Jailbreak kontrolü, uygulama
container'ı dışındaki yolların dosya metadata'sını okur; bu, Apple'ın
**file-timestamp** required-reason API kategorisine girer. Dolayısıyla çalışma
zamanı korumasını etkinleştirirseniz — `StringGuard.framework`'ü linkleyerek ya
da guard kütüphanesini [3.4 bölümündeki](#34-ios-frameworkün-linklenmesi) gibi
kendi framework'ünüzden export ederek — uygulamanızın
`PrivacyInfo.xcprivacy` dosyasının o kategori için bir
`NSPrivacyAccessedAPITypes` girdisine ihtiyacı olur.

Gönderimden önce güncel gerekçe kodu yönlendirmesini bizden isteyin. Apple
required-reason listesini zaman zaman değiştiriyor ve doğru bildirim
StringGuard kadar sizin uygulamanıza da bağlı; bu yüzden bir kodu bu belgeye
dondurmak yerine yönlendirmeyi güncel tutuyoruz. Bildirilmemiş bir
required-reason API genellikle reddedilme değil, gönderimde `ITMS-91053` uyarısı
üretir; yani bu, gönderimden önce çözülecek bir iş, bir engel değil.

Flutter plugin'inin kendi durumu var: App Attest anahtar kimliğini
`UserDefaults` içinde
tek bir değer olarak okur ve yazar. `UserDefaults` bir required-reason API'dir
ve bildirilmemiş kullanım gönderimde `ITMS-91053` uyarısı üretir. Plugin kendi
`PrivacyInfo.xcprivacy` dosyasını taşır ve
`NSPrivacyAccessedAPICategoryUserDefaults` kategorisini `CA92.1` gerekçesiyle
bildirir; bu nedenle uygulamanızın manifest'inde ayrıca bir bildirim gerekmez.
Bakınız
[Describing use of required reason API](https://developer.apple.com/documentation/bundleresources/privacy_manifest_files/describing-use-of-required-reason-api).

## 10. Mevcut kod tabanının taşınması

1. **Eklentiyi uygulayın.** [3. bölümdeki](#3-stringguardın-uygulanması) adımları
   izleyin ve `enabled = true` ile bir kez build alın. Obfuscate-all varsayılan
   olduğundan, uygun olan her şey bu ilk build'de kod değişikliği olmadan
   obfuscate edilir.

2. **Release build alın; test suite'inizi ve smoke test'lerinizi çalıştırın.**
   Koruma kuralları `const val` ve annotation argümanlarını kapsar. Uygulamanız
   bir runtime string literal'ini alışılmadık bir yerde kimliğine göre okuyorsa,
   örneğin bir literal üzerinde reflection yapıyor veya literal ile anahtarlanmış
   elle yazılmış bir resource lookup kullanıyorsa, o literal'i iki yoldan biriyle
   hariç tutun:
   - ilgili kapsamı `@DontObfuscate` ile işaretleyin, veya
   - bir `keepMatching` ya da `keepPackage` kuralı ekleyin.

3. **`minLength` değerini yükseltin.** Örneğin `4` değeri, önemsiz literal'leri
   atlar ve çıktıyı küçültür.

4. **Düz metin sızıntısı kontrolünü CI'a ekleyin.**
   [7. bölümdeki](#7-binary-doğrulaması) kontrol, düz metin olarak eklenen yeni
   bir sırrın otomatik olarak yakalanmasını sağlar.

5. **Opt-in modeli tercih ediyorsanız ona geçin.** `obfuscateAll = false` yapın
   ve hassas birkaç kapsamı `@Obfuscate` ile işaretleyin.

Aşağıdakiler olduğu gibi güvenlidir ve herhangi bir işlem gerektirmez: Retrofit
ve Ktor endpoint annotation'ları, `@SerialName` ile tanımlanan `@Serializable`
alan adları, `const val` olarak tanımlanan log tag'leri ve annotation argümanı
kullanan dependency injection qualifier'ları.

## 11. Sorun giderme

**`Incompatible Gradle JVM version … supports Java 1.8 and 24` (Android Studio).**
Android Studio'nun gömülü JBR'si JDK 25 olabilir; Gradle 8.14 bunu kabul etmez.
Gradle JDK'sını 17–24 aralığındaki bir sürüme ayarlayın:

- *Settings → Build, Execution, Deployment → Build Tools → Gradle → Gradle JDK* yolunu izleyip 17–21 aralığında bir JDK seçin veya ekleyin, **ya da**
- `gradle.properties` içinde `org.gradle.java.home=/path/to/jdk21` tanımlayın (ve/veya IDE'nin `#GRADLE_LOCAL_JAVA_HOME` seçeneğinin okuduğu `.gradle/config.properties` içinde `java.home=…` tanımlayın),

ardından **Sync Project with Gradle Files** komutunu çalıştırın. *"Apply
compatible Gradle JDK configuration"* düğmesine ne seçtiğini kontrol etmeden
basmayın; uygun olmayan eski bir JDK seçebilir.

**`Plugin [id: 'com.stringguard'] was not found`.** Lisansınızın bildirdiği depo
`pluginManagement.repositories` içinde yok ya da kimlik bilgileri
yapılandırılmamış ([2.1 bölümü](#21-repository-tanımları)).

**`Could not find com.stringguard:runtime` / `:annotations`.** Aynı depo
`dependencyResolutionManagement.repositories` içinde yok. Her iki repository
bloğu da ona ihtiyaç duyar, yalnızca `pluginManagement` değil.

**Kotlin sürümü yükseltildikten sonra compiler plugin sürüm veya ABI hatası.**
Compiler plugin belirli bir Kotlin sürümüne, 2.2.0'a karşı derlenmiştir.
Projenizin Kotlin sürümünü bununla eşleşmiş tutun veya kendi sürümünüze karşı
derlenmiş bir StringGuard build'i kullanın.

**Obfuscate edilmesini beklediğiniz bir literal hâlâ düz metin.** Şu sırayla
kontrol edin: literal bir `const val` mi; bir annotation argümanı mı;
`minLength` değerinden kısa mı; bir `@DontObfuscate` veya `@KeepPlain` kapsamı
içinde mi; bir `keepRules` kalıbıyla eşleşiyor mu; `enabledForDebug = false` ile
alınmış bir debug build'inde mi.

**Bir string obfuscate edildi ama çalışma anında okunabilir olması gerekiyor
(reflection).** O kapsamı `@DontObfuscate` ile veya bir `keepRules` girdisiyle
hariç tutun.

## 12. SSS

**Herhangi bir şeyi annotate etmek zorunda mıyım?** Hayır. Obfuscate-all
varsayılandır. `@DontObfuscate` ve `@KeepPlain` annotation'ları hariç tutma
içindir; `@Obfuscate` ise yalnızca isteğe bağlı opt-in modunda kullanılır.

**R8 veya ProGuard olmadan çalışır mı?** Evet. StringGuard R8'den bağımsızdır ve
Kotlin compiler'ı içinde çalışır. İkisi birbirini tamamlar ve birlikte
kullanılabilir: R8 yeniden adlandırma ve küçültme yapar, StringGuard string
içeriklerini şifreler.

**Uygulamayı yavaşlatır mı?** Şifre çözme literal başına yapılır, küçüktür ve
senkrondur. Tipik sır ve yapılandırma string'leri için maliyet ihmal
edilebilir düzeydedir. Ölçülebilir bir fark görürseniz çok büyük veya sıcak
döngü içindeki literal'leri `minLength` ya da `keepRules` ile hariç tutun.

**Bu gerçek bir güvenlik önlemi mi?** StringGuard statik analizin maliyetini
yükseltir; sıradan `strings` taramasını, decompile denemelerini ve otomatik
tarayıcıları etkisiz kılar. Kriptografik gizlilik sağlamaz: şifre çözme cihazda
gerçekleştiği için anahtar cihazda türetilebilir. Hiçbir koşulda sızmaması
gereken bir değer sunucuda tutulmalıdır. Güvenlik modelinin tamamı README'de
anlatılmıştır.

**Multiplatform'da iOS ve Wasm gerçekten kapsanıyor mu?** Evet. Runtime saf
Kotlin'dir ve `commonMain` içine eklenir; aynı şifreli payload JVM,
Kotlin/Native (iOS backend), JS ve Wasm üzerinde birebir aynı şekilde çözülür.
Tek bir lisans her hedefi kapsar; hedefler için ayrı bir yetki gerekmez.

**Şifreleme algoritmasını ChaCha20 olarak değiştirebilir miyim?** Evet.
`stringGuard { }` bloğunda
`algorithm = StringGuardAlgorithm.CHACHA20_POLY1305` değerini verin. Runtime,
her hedefte hem AES-GCM hem de ChaCha20-Poly1305 payload'larını çözer; bu nedenle
algoritmalar arasında serbestçe geçiş yapabilirsiniz. Varsayılan AES-GCM'dir.

**Anahtarın uygulama imzasına bağlanması (tamper binding) nasıl yapılır?** Bu
özellik `tamperBinding` adıyla sunulur (yalnızca Android). Kurulum, Play App
Signing notu ve mekanizmanın sınırları için README'deki
[İmza bütünlüğüne bağlama](../README.tr.md#i̇mzaya-bağlama-android) bölümüne bakın.
