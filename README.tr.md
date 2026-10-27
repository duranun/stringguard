# StringGuard

**[English](README.md)** · **Türkçe**

StringGuard, Kotlin için compile-time string obfuscation aracıdır. Tek bir
Gradle plugin'i KMP ve Compose Multiplatform projelerinin yanı sıra KMP
kullanmayan Android ve JVM projelerini de kapsar.

Varsayılan olarak projenizdeki uygun her string literal, Kotlin K2 compiler
plugin'i tarafından build sırasında şifrelenir ve runtime'da onu çözen bir
çağrıyla değiştirilir. Plain text değer binary'ye hiç girmez; APK, JAR veya
framework üzerinde `strings` taraması ya da basit bir decompile hiçbir şey
bulamaz.

```kotlin
val apiKey = "sk_live_51H...Z"   // siz bunu yazarsınız
```

Binary'ye giden, değeri runtime'da anlamsız bir byte dizisinden çözen bir
çağrıdır. Hiçbir plain text yazılmaz.

- Tüm Kotlin target'ları için tek plugin: JVM/Desktop, Android (KMP ve klasik),
  iOS/Native, JS, Wasm. Koruma her target'ta aynı şekilde uygulanır, davranış
  platforma göre değişmez.
- Başlamak için annotation gerekmez. Obfuscate-all varsayılan olarak açıktır;
  kodu dahil etmezsiniz, hariç tutarsınız.
- Compile-time sabiti olarak kalması gereken literal'lere (`const val`
  initializer'ları, annotation argümanları) hiç dokunulmaz. Retrofit,
  kotlinx.serialization ve Room çalışmaya devam eder.

## Gereksinimler

| Bileşen | Gereksinim |
|---|---|
| JDK | 17+ (21 önerilir). Gradle Java 8–24 arasını kabul eder; JDK 25 ve 26 reddedilir, bkz. [Sorun giderme](#sorun-giderme). |
| Gradle | 8.x (8.14 ile test edildi) |
| Kotlin | 2.2.0 (compiler plugin'i Kotlin sürümüne bağlıdır) |
| AGP | 8.x (Android için) |

> **Erişim:** StringGuard lisanslı bir üründür. Lisansınız artifact'ları ve
> onları çekeceğiniz repository'yi kapsar; ikisi de anahtarla birlikte gelir.

## Kurulum

Burası kısa yol. Platform bazında kurulum, mevcut bir projeye geçiş, CI,
lisanslama ve sorun giderme için
[docs/INTEGRATION.tr.md](docs/INTEGRATION.tr.md) dokümanına bakın.

### Adım 1: repository tanımı

Projenin `settings.gradle.kts` dosyasında:

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

Plugin `annotations` ve `runtime` kütüphanelerini kendisi eklediği için
repository iki blokta da olmalı. Birini unutursanız
[Sorun giderme](#sorun-giderme) bölümündeki resolution hatalarından birini
alırsınız.

### Adım 2: plugin'i ekleme

Plugin'i modülünüzün zaten kullandığı Kotlin plugin'inin yanına ekleyin. Sıra
önemli değil; StringGuard, Kotlin plugin'i ne zaman uygulanırsa ona bağlanır.
`annotations` ve `runtime` dependency'lerini de proje tipine göre doğru
configuration'a veya source set'e kendisi ekler, ayrıca bir şey tanımlamanız
gerekmez.

<details open>
<summary>Klasik Android (KMP olmayan)</summary>

```kotlin
// app/build.gradle.kts
plugins {
    id("com.android.application")
    kotlin("android")
    id("com.stringguard") version "1.0.0"
}
```
</details>

<details>
<summary>JVM / Desktop</summary>

```kotlin
plugins {
    kotlin("jvm") version "2.2.0"
    id("com.stringguard") version "1.0.0"
}
```
</details>

<details>
<summary>Kotlin Multiplatform / Compose Multiplatform</summary>

```kotlin
plugins {
    kotlin("multiplatform") version "2.2.0"
    id("com.stringguard") version "1.0.0"
}
```
Runtime `commonMain`'e eklenir; böylece tüm target'lar (Android, iOS/Native,
JS, Wasm, Desktop) aynı build ile kapsanır.
</details>

### Adım 3: lisans anahtarı

StringGuard lisans anahtarı ister; geçerli anahtar yoksa build başarısız olur.
Anahtarı plugin'i uygulayan modüle ekleyin:

```kotlin
stringGuard {
    licenseKey = "<token'ınız>"      // ya da token'ı içeren bir dosyanın yolu
}
```

Token'ı version control'e koymayın. Genelde environment'tan okunur:

```kotlin
stringGuard {
    licenseKey = providers.environmentVariable("STRINGGUARD_LICENSE").getOrElse("")
}
```

Anahtar eksikse veya süresi dolmuşsa ne olacağı [Lisanslama](#lisanslama)
bölümünde.

### Adım 4: build

Obfuscate-all varsayılan olarak açık; korumalı bir build için annotation
gerekmez.

```bash
./gradlew assembleRelease   # Android
./gradlew build             # JVM / KMP
```

[Hiçbir zaman obfuscate edilmeyenler](#hiçbir-zaman-obfuscate-edilmeyenler)
listesindekiler ve sizin hariç tuttuklarınız dışında çıktıdaki tüm string
literal'ler şifrelenir.

## Dokümanlar

| Doküman | İçerik |
|---|---|
| [docs/INTEGRATION.tr.md](docs/INTEGRATION.tr.md) | Adım adım entegrasyon: platform bazında kurulum, mevcut projeye geçiş, CI, lisanslama, sorun giderme. |
| [Attestation](docs/INTEGRATION.tr.md#10-attestation) | Uygulamanıza eklemek, servisi çalıştırmak ve policy belirlemek. |
| [Backend doğrulaması](docs/INTEGRATION.tr.md#104-backendde-karar-verme) | API'nizin session token'ı nasıl doğrulayacağı, sonucu nasıl okuyacağı ve bir request'i cihaza nasıl bağlayacağı. |
| [docs/DATA_HANDLING.md](docs/DATA_HANDLING.md) | Attestation servisinin hangi verileri işlediği, sakladığı ve hiç dokunmadığı. DPA için girdi. |
| [swift/README.md](swift/README.md) | Doğrudan Swift ile yazılmış uygulamalar için string obfuscation. |
| [SECURITY.md](SECURITY.md) | Güvenlik açığı bildirme. |

## Güvenlik modeli ve sınırlar

Cihaz üzerinde obfuscation kriptografik gizlilik değildir. Decrypt cihazda
yapıldığı için anahtar da orada elde edilebilir. Debugger, Frida veya runtime
hook gibi dinamik analiz araçları kullanan bir saldırgan korunan değerleri
runtime'da ele geçirebilir.

StringGuard'ın sağladığı, statik analizin maliyetini yükseltmektir. `strings`
taramasını, basit decompile'ı ve otomatik secret scanner'ları boşa çıkarır;
sızıntıların çoğu bunlarla olur. DexGuard gibi ticari araçlarla aynı yaklaşım:
statik obfuscation, buna ek olarak opsiyonel bir runtime self-protection (RASP)
katmanı ve uygulama imzasına bağlama; ikisi de aşağıda anlatılıyor. Binary'de
okunabilir bir decryption key tutulmaz.

StringGuard'ı caydırıcı ve IP koruma katmanı olarak düşünün, asla sızmaması
gereken bir secret için kasa olarak değil. Hiçbir şekilde ifşa olmaması gereken
bir değerin yeri client binary'si değil, sunucudur.

Güvenlik açıkları [SECURITY.md](SECURITY.md) üzerinden bildirilir.

## Neyin obfuscate edileceğini belirleme

### Varsayılan: her şey

Hiç ayar yapmazsanız, uzunluğu en az `minLength` olan tüm `String` literal'ler
proje genelinde obfuscate edilir. Kural yalnızca hariç tutmak için yazılır.

### `@DontObfuscate` ile kapsam dışı bırakma

Bir dosyayı, class'ı, object'i, fonksiyonu, property'yi veya constructor'ı
işaretlerseniz içindeki literal'ler plain kalır. Reflection key'leri, resource
isimleri, log tag'leri veya bir aracın değerine bakarak okuduğu her şey için
kullanışlıdır.

```kotlin
import com.stringguard.annotations.DontObfuscate

@DontObfuscate
object PublicConstants {
    val featureFlagKey = "enable_new_home"   // okunabilir kalır
}
```

`@KeepPlain`, `@DontObfuscate` ile birebir aynıdır; hangisi daha iyi okunuyorsa
onu kullanın.

### `keepRules` ile pattern bazlı hariç tutma

```kotlin
// build.gradle.kts
stringGuard {
    minLength = 4                       // çok kısa literal'leri atla (varsayılan 1)
    keepRules {
        keepPackage("com.myapp.resources.*")   // bu package ve alt package'ları plain kalır
        keepMatching("^https://cdn\\.")        // bu regex'e uyan literal'ler plain kalır
    }
}
```

### Hiçbir zaman obfuscate edilmeyenler

Aşağıdakiler compile-time sabiti olmak zorunda oldukları için otomatik atlanır;
obfuscate edilseler kod derlenmez:

- `const val` initializer'ları, örneğin `const val TAG = "MyScreen"`
- Annotation argümanları, örneğin `@SerialName("user_id")`,
  `@GET("users/{id}")`, `@Deprecated("…")`
- Annotation class gövdeleri, yani default değerleri

StringGuard'ı tüm projede rahatça açabilmenizin sebebi bu: Retrofit
endpoint'leri, `@Serializable` alan isimleri ve benzerleri olduğu gibi çalışır.

### Opt-in modu

Yalnızca işaretlediklerinizi obfuscate etmek için obfuscate-all'u kapatın ve
`@Obfuscate` ile istediğiniz kapsamları dahil edin:

```kotlin
stringGuard { obfuscateAll = false }
```
```kotlin
import com.stringguard.annotations.Obfuscate

@Obfuscate
object Secrets {
    val apiKey = "sk_live_..."    // obfuscate edilir; projenin geri kalanı plain kalır
}
```

## Konfigürasyon referansı

```kotlin
stringGuard {
    enabled = true                 // ana şalter; false = plugin hiçbir şey yapmaz
    obfuscateAll = true            // true (varsayılan): tüm proje; false: yalnızca @Obfuscate kapsamları
    minLength = 1                  // bundan kısa literal'ler plain kalır
    enabledForDebug = false        // debug build type'ları da obfuscate et (kapalı = hızlı, grep'lenebilir dev build'leri)
    licenseKey = "<token>"         // ZORUNLU: lisans metni ya da lisans dosyasının yolu

    guard = false                  // @GuardWith reveal noktalarında runtime self-protection
    tamperBinding = false          // Android: string'ler yalnızca şu sertifikalardan biriyle...
    signingCertSha256("AB:CD:...") // ...imzalanmış bir APK'da çözülür (tekrarlanabilir)

    // AEAD algoritması (varsayılan AES_GCM). ChaCha20-Poly1305 da desteklenir:
    algorithm = StringGuardAlgorithm.CHACHA20_POLY1305
    keepRules {
        keepPackage("com.app.res.*")
        keepMatching("^android\\.")
    }
}
```

| Ayar | Varsayılan | Etkisi |
|---|---|---|
| `enabled` | `true` | false ise plugin tamamen kapanır. |
| `obfuscateAll` | `true` | Tüm projeyi ya da yalnızca `@Obfuscate` kapsamlarını obfuscate eder. |
| `minLength` | `1` | N karakterden kısa literal'leri atlar. |
| `enabledForDebug` | `false` | Varsayılan olarak debug build'ler hızlı ve grep'lenebilir geliştirme için plain bırakılır. Test compilation'ları her zaman atlanır. |
| `licenseKey` | `""` | **Zorunlu.** Offline lisans: lisans metni ya da onu içeren dosyanın yolu (göreli yollar modül dizinine göre çözülür). Gömülü public key ile, ağa çıkmadan doğrulanır. Eksik, bozuk veya süresi dolmuş anahtar **build'i durdurur**. Bkz. [Lisanslama](#lisanslama). |
| `guard` | `false` | `@GuardWith` kapsamlarındaki reveal noktalarına runtime self-protection ekler. Bkz. [Runtime guard](#runtime-guard-rasp). |
| `tamperBinding` | `false` | Yalnızca Android. Android string'lerini imza sertifikalarınıza bağlar. Bkz. [Tamper binding](#tamper-binding-android). |
| `signingCertSha256(...)` | yok | `tamperBinding` için izin verilen imza sertifikası SHA-256 fingerprint'leri. `tamperBinding` açıksa zorunlu. |
| `algorithm` | `AES_GCM` | AEAD algoritması: `AES_GCM` (varsayılan) veya `CHACHA20_POLY1305`. Runtime ikisini de her target'ta çözer. |

### Debug ve release build'ler

Test kodu hiçbir zaman obfuscate edilmez. Debug build type'ları
`enabledForDebug = true` yapılmadıkça plain kalır; böylece geliştirme döngüsü
hızlı, log'lar grep'lenebilir olur. Release build'ler obfuscate edilir.

## Lisanslama

StringGuard'ın tek bir özellik seti ve tek bir lisans türü var. Geçerli bir
anahtar bu README'de anlatılan her şeyi tüm target'larda açar: free, trial ya
da ara bir seviye yoktur ve lisanslı bir build'den hiçbir şey eksik bırakılmaz.

Doğrulama offline yapılır. Anahtar, plugin'e gömülü bir public key ile
configuration aşamasında, ağa çıkmadan kontrol edilir; CI'da ve internete kapalı
ortamlarda da çalışır.

**Geçerli lisansı olmayan build başarısız olur.** Eksik, bozuk veya süresi
dolmuş bir anahtar configuration'ı açıklayıcı bir hatayla durdurur:

```
> StringGuard requires a license key. Set stringGuard.licenseKey to your license
  token, or to the path of a file holding it. To build this module without
  StringGuard instead, set stringGuard.enabled = false.
```

Bu bilinçli bir tercih. Alternatifi, yani devam edip daha az obfuscate etmek,
yeşil bir build'in içinde okunabilir secret'larla ship edilmesi demektir; bir
string obfuscator'ın asla yol açmaması gereken tek sonuç da budur. Gerçekten
obfuscate edilmemesi gereken bir modül `stringGuard { enabled = false }` ile
dışarıda kalır ve o zaman anahtar gerekmez.

Başarılı bir build lisansı bir kez log'lar:

```
StringGuard: license verified (holder=Acme Inc, expires=2027-04-01)
```

## Tamper binding (Android)

Tamper binding, obfuscate edilmiş her Android string'ini uygulamanızın imza
sertifikasına bağlar. Listelediğiniz sertifikalardan biriyle imzalanmış bir
APK'da string'ler normal şekilde çözülür. Biri APK'nızı kendi anahtarıyla
yeniden imzalarsa (repackaging'in olağan adımı) bağlı her string makul görünen
anlamsız bir değere çözülür. Uygulama çökmez, dolayısıyla kontrol saldırgana
belli edilmez.

```kotlin
stringGuard {
    tamperBinding = true
    signingCertSha256("AB:CD:EF:...")   // Play App Signing key'i: Play Console → Test and release → App integrity
    signingCertSha256("12:34:56:...")   // upload veya debug key, o key'le imzalanan build'lerin çalışması gerekiyorsa
}
```

Fingerprint'i şunlardan biriyle alabilirsiniz:

```bash
apksigner verify --print-certs app-release.apk   # "certificate SHA-256 digest"
keytool -list -v -keystore release.jks           # "SHA256:"
```

- Play App Signing kullanıyorsanız kullanıcıların kurduğu APK sizin upload
  key'inizle değil Google'ın key'iyle imzalanır. Play App Signing sertifikasını
  listeleyin, yoksa production kurulumlar anlamsız string'ler görür.
- `tamperBinding = true` olup hiç sertifika yoksa veya biri hatalıysa build
  configuration aşamasında durur; yanlış ayarlanmış bir uygulama kazara ship
  edilemez.
- Yalnızca Android compilation'ları bağlanır. Aynı projedeki iOS, Desktop, JS
  ve Wasm target'ları etkilenmez.
- Debug build'ler varsayılan olarak obfuscate edilmez. `enabledForDebug = true`
  ise debug sertifikasını da listeleyin.
- Fingerprint **64 hex karakterdir, iki nokta ile ayrılabilir**; en fazla
  **16 sertifika** listeleyebilirsiniz. Hatalı veya eksik bir değer build'i
  configuration aşamasında durdurur.
- Release variant'ın JVM unit test'leri cihaz dışında çalışır, okunacak bir APK
  yoktur. Plugin bu testlerde bağlı string'leri okunabilir bırakır, test
  suite'iniz çalışmaya devam eder; gerçek cihazda yalnızca kurulu APK'nın
  sertifikası geçerlidir.
- **Sertifika değiştirmek aşamalı yapılır.** Yeni fingerprint'i ekleyip o
  build'i önce yayınlayın. Eskisini ancak ona bağlı hiçbir kurulumun çalışması
  gerekmediğinde kaldırın; erken kaldırırsanız o kurulumların hepsi bozulur.
- Özel bir Android izni, API'si veya `Context` gerekmez.
- Sınır: Bu, kodu değiştirmeden yapılan yeniden imzalamayı durdurur. Kodu
  patch'leyen bir saldırgan aşabilir, çünkü sertifikanızın fingerprint'i kendi
  APK'nızda herkese açıktır.

## Runtime guard (RASP)

```kotlin
stringGuard { guard = true }   // guard runtime'ını modüle ekler
```
```kotlin
import com.stringguard.annotations.GuardWith

@GuardWith
object PaymentKeys {
    val merchantKey = "mk_live_..."
}
```

`@GuardWith` kapsamındaki korumalı bir değer yalnızca process temiz
göründüğünde açılır. Debugger veya hooking framework varsa gerçek değer hiç
çözülmez; yerine aynı uzunlukta, makul görünen bir sahte değer döner. Uygulama
çalışmaya devam eder ve kontrol, onu kurcalayan kişiye belli edilmez.

Platforma göre tespit edilenler:

| Sinyal | Android | JVM / Desktop | iOS | macOS | JS / Wasm |
|---|---|---|---|---|---|
| Debugger bağlı | ✓ | ✓ | ✓ | ✓ | — |
| Hooking framework | ✓ | ✓ | ✓ | ✓ | — |
| Root / jailbreak | ✓ | ✓ (Linux) | ✓ (yalnızca cihaz) | — | — |

Güvenebileceğiniz davranışlar ve size bıraktığı kararlar:

- **Root'lu veya jailbreak'li cihaz raporlanır ama değerler bozulmaz**, çünkü
  birçok gerçek kullanıcı telefonunu root'lar. Bu cihazları reddetmek
  istiyorsanız açılışta `Guard.rootIsSuspicious = true` yapın. Bu, gerçek
  kullanıcıları geri çevirmek anlamına gelen bir ürün kararıdır; bilerek verin.
- **Debugger veya hook bir kez görüldüyse değerler process boyunca bozuk
  kalır**, araç ayrılsa bile.
- **Tespitler bize değil size raporlanır.** `Detections.onDetection`, her sinyal
  kategorisi ilk kez tetiklendiğinde çağrılır; `Detections.all()` o ana kadar
  görülenlerin hepsini verir. Bir sinyal callback'iniz kurulmadan önce
  tetiklenebileceği için açılışta onu da okuyun. StringGuard ağa çıkmaz, hiçbir
  şey toplamaz; kaydın nereye gideceğine kendi telemetry'niz karar verir.

  ```kotlin
  Detections.onDetection = { category ->
      analytics.log("stringguard.detection", mapOf("category" to category.name))
  }
  Detections.all().forEach { /* açılış sırasında tetiklenenler */ }
  ```

  Bir event yalnızca kategori taşır (`DEBUGGER`, `HOOKING_FRAMEWORK`,
  `ROOT_OR_JAILBREAK`), eşleşen dosyayı ya da kütüphaneyi taşımaz. Callback
  sinyali gören thread'de çalışır, hemen dönün; içinden fırlatılan exception'lar
  yutulur, bir reveal'ı bozmaz.

  Root, değerleri bozmasa da raporlanır; böylece kullanıcılarınızın ne kadarının
  root'lu cihaz kullandığını, onlara saldırgan muamelesi yapmadan görebilirsiniz.
  Bir şeyi zorunlu kılmadan önce veri toplayın: hooking framework raporlayan tek
  bir cihaz pek bir şey ifade etmez, tek bir release'te dört yüz cihaz ise
  harekete geçmeniz gereken bir durumdur.
- **Tespit process bazındadır.** Bir kontrol yalnızca onu yapan process'i
  görür. Uygulamanız secret'ları başka bir process'te açıyorsa
  (`android:process=`, isolated service, ayrı bir native process),
  `@GuardWith` kapsamları ve `Guard.report()` çağrıları da o process'te
  olmalıdır; ana process'teki kontrol diğerleri hakkında hiçbir şey söylemez.
  `Guard.rootIsSuspicious` da process bazındadır, her birinde ayrıca ayarlayın.
  Attestation bundan etkilenmez.
- **Kontroller kısa süre cache'lenir**, aynı ekrandaki çok sayıda korumalı
  string tek bir kontrole mal olur.
- **Sınırlar.** Her RASP kontrolü, cihazın tam kontrolünü elinde tutan kararlı
  bir saldırgan tarafından aşılabilir. Dinamik analizin maliyetini yükseltir,
  garanti değildir. Sunucu tarafındaki bir secret'ın yerine değil, onun önünde
  ek bir katman olarak düşünün.

### R8 ve minification

StringGuard artifact'ları kendi consumer ProGuard kurallarıyla gelir; minify
eden bir Android uygulamasında elle bir şey yapmanız gerekmez, R8 bunları
otomatik alır. Obfuscate edilmiş string'ler minify edilmiş build'de doğru
çözülür.

R8 ile StringGuard birbirini tamamlar ve birlikte kullanılmak üzere
tasarlanmıştır: R8 isimleri değiştirip küçültür, StringGuard da R8'in okunabilir
bıraktığı string içeriklerini şifreler.

## Plain text kalmadığını doğrulama

Korunan literal'ler derlenmiş çıktıda yer almaz ve bu, desteklenen her backend'de
gösterilmeden hiçbir StringGuard sürümü yayınlanmaz. JVM ve Desktop'ta doğrudan
görebilirsiniz:

```bash
./gradlew build
javap -c -p -classpath build/libs/<your>.jar com.example.Config
# secret literal'ler yok
```

## Sorun giderme

**"Incompatible Gradle JVM version … supports Java 1.8 and 24" (Android
Studio).** Android Studio'nun bundled JBR'ı JDK 25 olabilir, Gradle 8.14 bunu
reddeder. Gradle JDK'yı 17 ile 24 arası bir JDK'ya çevirin:
`Settings → Build, Execution, Deployment → Build Tools → Gradle → Gradle JDK`
ya da `gradle.properties` içinde `org.gradle.java.home=/path/to/jdk21`. Sonra
*Sync Project with Gradle Files* çalıştırın.

**"Plugin `com.stringguard` not found."** Lisansınızda belirtilen repository
`pluginManagement.repositories` içinde yok. Bkz.
[Adım 1](#adım-1-repository-tanımı).

**"Could not find com.stringguard:runtime" / ":annotations".** Aynı repository
`dependencyResolutionManagement.repositories` içinde yok. İki blokta da olmalı.

**Şifrelenmesini beklediğim bir literal hâlâ plain text.** Büyük ihtimalle bir
`const val`, bir annotation argümanı, `minLength`'ten kısa,
`@DontObfuscate`/`@KeepPlain` kapsamında, bir `keepRules` pattern'ine uyuyor ya
da `enabledForDebug = false` iken bir debug build'de.

**Retrofit / kotlinx.serialization annotation değerleri değişti mi?** Değişmez.
Annotation argümanları tasarım gereği hiçbir zaman obfuscate edilmez.

**`tamperBinding` açıkken kurulu uygulama bozuk string'ler gösteriyor.** Kurulu
APK, listelemediğiniz bir sertifikayla imzalanmış. Play App Signing
kullanıyorsanız bu Google'ın key'idir: fingerprint'ini Play Console → App
integrity'den ekleyin. APK'yı `apksigner verify --print-certs` ile kontrol edin.

**"tamperBinding = true requires at least one signingCertSha256".**
Fingerprint'leri [Tamper binding](#tamper-binding-android) bölümündeki gibi
ekleyin.

**"StringGuard requires a license key."** `stringGuard.licenseKey` ayarlı değil
ya da boş bir dosyayı gösteriyor. Lisans token'ınızı ya da onu içeren dosyanın
yolunu verin (göreli yollar modül dizinine göre çözülür). Hiç obfuscate
edilmemesi gereken bir modül bunun yerine `stringGuard { enabled = false }`
kullanır, anahtar gerekmez.

**"StringGuard license is not valid: ..."** Anahtar bir StringGuard lisans
token'ı değil, eksik kopyalanmış ya da bu StringGuard build'i için verilmemiş.
Mesajdaki sebep hangisi olduğunu söyler. `license format v1 is no longer
supported` görüyorsanız anahtar eski formattadır; yenisini isteyin.

**"StringGuard license '...' expired on ...".** Lisansı yenileyip
`stringGuard.licenseKey`'i güncelleyin. Build'lerin korumasız devam etmesine
izin verilmez, bunu atlatan bir ayar yoktur.

## Attestation (opsiyonel)

Attestation, her uygulama session'ı için backend'inize isteğin sağlam bir
cihazdaki gerçek uygulamanızdan gelip gelmediğini söyler. Sonucu backend'iniz
okur ve neye izin vereceğine kendisi karar verir.

- **Android ve iOS.** iOS'ta uygulamanın provisioning profile'ında App Attest
  capability'si ve fiziksel bir cihaz gerekir.
- **Standart bir sonuç.** Backend'iniz onu zaten kullandığı kütüphaneyle, bir
  JWT olarak doğrular.
- **Request binding.** Hassas request'ler cihazda imzalanabilir; kopyalanan bir
  session token başka hiçbir yerde işe yaramaz.
- **Varsayılan olarak hiçbir şey engellenmez.** Her sonucun neye izin vereceğine
  işlem bazında siz karar verirsiniz ve bunu uygulama güncellemesi olmadan
  değiştirebilirsiniz.
- **Self-hosted.** Servis kendi altyapınızda çalışır; lisansları ve cihazları
  yönetmek için bir dashboard'u ve bir API'si vardır.

Uygulamanıza eklemek ve servisi çalıştırmak için
[docs/INTEGRATION.tr.md](docs/INTEGRATION.tr.md#10-attestation) dokümanına bakın.

## Lisans

StringGuard ticari, kapalı kaynak bir yazılımdır. Açık kaynak değildir ve
kaynak kod lisansı verilmez.

StringGuard ile build almak için ticari bir lisans anahtarı gerekir. Tek bir
özellik seti vardır, free tier yoktur. Geçerli bir anahtarla ticari kullanım
dahil her amaçla build alabilir, runtime'ı kendi uygulamanızın içinde ship
edebilir ve o uygulamayı app store'lar üzerinden ve müşterilerinize
dağıtabilirsiniz. Uygulamanızın yayınlanmış bir sürümünde zaten ship edilmiş
runtime bileşenleri, anahtarınızın süresi dolduktan sonra da son kullanıcılarınız
için lisanslı kalır. Artifact'lar olduğu gibi yeniden dağıtılabilir, örneğin
package registry'leri ve mirror'ları tarafından.

Tam koşullar [LICENSE](LICENSE) dosyasında. Üçüncü taraf bileşenler ve
lisansları [NOTICE](NOTICE) dosyasında listelenir.
