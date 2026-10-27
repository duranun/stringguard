# StringGuard

**[English](README.md)** · **Türkçe**

StringGuard, Kotlin için derleme zamanında çalışan bir metin gizleme (string
obfuscation) çözümüdür. Tek bir Gradle eklentisi hem KMP ve Compose
Multiplatform projelerini hem de standart (KMP kullanmayan) Android ve JVM
projelerini kapsar.

Varsayılan olarak projedeki uygun metin sabitleri, Kotlin K2 derleyici eklentisi
tarafından derleme sırasında şifrelenir ve çalışma zamanında bu değerleri çözen
çağrılarla değiştirilir. Açık metin, derlenmiş ikili dosyada yer almaz. Bu
nedenle APK, JAR veya framework üzerinde yapılan bir `strings` taraması ya da
yüzeysel bir tersine inceleme sonuç vermez.

```kotlin
val apiKey = "sk_live_51H...Z"   // sizin yazdığınız kod
```

İkili dosyaya ulaşan karşılık, metni çalışma anında çözen bir çağrıdır. Çıktıya
hiçbir açık metin yazılmaz.

- Her Kotlin hedefi için tek eklenti: JVM/Desktop, Android (hem KMP hem klasik),
  iOS/Native, JS ve Wasm. Koruma her hedefte aynı biçimde uygulanır; davranış
  platforma göre değişmez.
- Başlamak için ek açıklama (annotation) gerekmez. Tüm uygun metinleri gizleme
  varsayılan olarak açıktır; kodu dahil etmek yerine hariç tutarsınız.
- Derleme zamanında sabit kalması gereken değerler (`const val` başlatıcıları,
  annotation argümanları) hiçbir zaman değiştirilmez. Böylece Retrofit,
  kotlinx.serialization ve Room çalışmaya devam eder.

## Gereksinimler

| Bileşen | Gereksinim |
|---|---|
| JDK | 17+ (21 önerilir). Gradle, Java 8 ile 24 arasındaki sürümleri kabul eder; JDK 25 ve 26 reddedilir, bkz. [Sorun giderme](#sorun-giderme). |
| Gradle | 8.x (8.14 ile test edildi) |
| Kotlin | 2.2.0 (derleyici eklentisi kullanılan Kotlin sürümüne sabitlenmiştir) |
| AGP | 8.x (Android için) |

> **Erişim:** StringGuard lisanslı bir üründür. Lisansınız hem çıktıları hem de
> onları çözeceğiniz depoyu kapsar; ikisi de anahtarla birlikte gelir.

## Kurulum

Bu bölüm hızlı yoldur. Platform bazlı kurulum, mevcut bir kod tabanına geçiş, CI
kontrolleri, lisanslama ve sorun giderme için
[docs/INTEGRATION.tr.md](docs/INTEGRATION.tr.md) belgesine bakın.

### Adım 1: depo yapılandırması

Eklentiyi kullanacak projenin `settings.gradle.kts` dosyasında:

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

Eklenti `annotations` ve `runtime` kütüphanelerini sizin için eklediğinden her
iki blok da depoya ihtiyaç duyar; birini atlamak
[Sorun giderme](#sorun-giderme) altındaki çözümleme hatalarından birine yol
açar.

### Adım 2: eklentinin uygulanması

Eklentiyi, modülünüzde halihazırda kullandığınız Kotlin eklentisinin yanına
ekleyin. Sıra önemli değildir; StringGuard, Kotlin eklentisi uygulandığı anda
kendisini ona bağlar. `annotations` ve `runtime` bağımlılıklarını da proje
türüne uygun configuration veya source set içine kendisi ekler, böylece ayrıca
tanımlamanız gereken bir şey kalmaz.

<details open>
<summary>Standart Android (klasik, KMP kullanmayan)</summary>

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
`runtime` bağımlılığı `commonMain` source set'ine eklenir; böylece her hedef
(Android, iOS/Native, JS, Wasm, Desktop) aynı build ile kapsanır.
</details>

### Adım 3: lisans anahtarı

StringGuard bir lisans anahtarı gerektirir; geçerli anahtarı olmayan derleme
kırılır. Anahtarı, eklentiyi uygulayan modüle ekleyin:

```kotlin
stringGuard {
    licenseKey = "<anahtarınız>"     // veya onu içeren bir dosyanın yolu
}
```

Anahtarı sürüm kontrolünden uzak tutun. Alışılmış yöntem onu ortamdan okumaktır:

```kotlin
stringGuard {
    licenseKey = providers.environmentVariable("STRINGGUARD_LICENSE").getOrElse("")
}
```

Anahtar eksik veya süresi dolmuşsa ne olduğu için bkz.
[Lisanslama](#lisanslama).

### Adım 4: derleme

Tüm uygun metinleri gizleme varsayılan olarak açık olduğundan, korumalı bir
çıktı almak için herhangi bir annotation eklemeniz gerekmez.

```bash
./gradlew assembleRelease   # Android
./gradlew build             # JVM / KMP
```

[Asla gizlenmeyenler](#asla-gizlenmeyenler) bölümünde listelenenler ve kendi
hariç tutma kurallarınızla kapsam dışında bıraktıklarınız dışında, çıktıdaki tüm
metin sabitleri şifrelenir.

## Belgeler

| Belge | İçerik |
|---|---|
| [docs/INTEGRATION.tr.md](docs/INTEGRATION.tr.md) | Tam entegrasyon kılavuzu: platform bazlı kurulum, mevcut kod tabanına geçiş, CI kontrolleri, lisanslama, sorun giderme. |
| [docs/ATTESTATION.md](docs/ATTESTATION.md) | Attestation: neyi kanıtlar, nasıl entegre edilir, politikası nasıl belirlenir. |
| [docs/DATA_HANDLING.md](docs/DATA_HANDLING.md) | Attestation servisinin hangi verileri işlediği, sakladığı ve hiç dokunmadığı. DPA girdisi. |
| [swift/README.md](swift/README.md) | Doğrudan Swift ile yazılmış uygulamalar için string obfuscation. |
| [SECURITY.md](SECURITY.md) | Güvenlik açığı bildirimi. |

## Güvenlik modeli ve sınırları

Cihaz üzerinde yapılan metin gizleme, kriptografik gizlilik sağlamaz. Çözme
işlemi cihazda gerçekleştiği için anahtar da cihazda elde edilebilir. Hata
ayıklayıcı, Frida veya çalışma zamanı kancaları gibi dinamik analiz araçlarını
kullanan bir saldırgan, korunan değerleri çalışma zamanında geri elde edebilir.

StringGuard'ın sağladığı şey, statik analizin maliyetini yükseltmektir.
`strings` taramasını, yüzeysel tersine incelemeyi ve otomatik gizli bilgi
tarayıcılarını etkisiz bırakır; ihlallerde en sık başvurulan yöntemler
bunlardır. Yaklaşım, DexGuard gibi ticari araçlarla aynı biçimdedir: statik
gizleme ile birlikte isteğe bağlı bir çalışma zamanı kendini koruma (RASP)
katmanı ve uygulama imzasına bağlama, ikisi de aşağıda anlatılan
özellikleridir. İkili dosyada okunabilir bir çözme anahtarı saklanmaz.

StringGuard'ı caydırıcı bir katman ve fikrî mülkiyet koruması olarak
değerlendirin; asla sızmaması gereken bir sırrı sakladığınız bir kasa olarak
değil. Hiçbir koşulda açığa çıkmaması gereken bir değer, istemci ikili
dosyasında değil sunucuda durmalıdır.

Güvenlik açıkları [SECURITY.md](SECURITY.md) üzerinden bildirilir.

## Hangi metinlerin gizleneceğini yönetme

### Varsayılan: tüm uygun metinler

Hiçbir yapılandırma yapılmadığında, uzunluğu en az `minLength` olan her `String`
sabiti proje genelinde gizlenir. Kural yazmanız yalnızca hariç tutmak
istedikleriniz için gerekir.

### `@DontObfuscate` ile kapsam hariç tutma

Bir dosyayı, sınıfı, object'i, fonksiyonu, property'yi veya constructor'ı
işaretleyerek o kapsamdaki metin sabitlerini açık bırakabilirsiniz. Reflection
anahtarları, kaynak adları, log etiketleri ve değeri birebir okunan diğer
metinler için kullanışlıdır.

```kotlin
import com.stringguard.annotations.DontObfuscate

@DontObfuscate
object PublicConstants {
    val featureFlagKey = "enable_new_home"   // okunabilir kalır
}
```

`@KeepPlain`, `@DontObfuscate` ile birebir aynı anlama gelen bir takma addır.
Kodunuzda hangisi daha anlaşılırsa onu kullanın.

### `keepRules` ile kalıba dayalı hariç tutma

```kotlin
// build.gradle.kts
stringGuard {
    minLength = 4                       // çok kısa metinleri atla (varsayılan 1)
    keepRules {
        keepPackage("com.myapp.resources.*")   // bu paket ve alt paketleri açık kalır
        keepMatching("^https://cdn\\.")        // bu regex ile eşleşen metinler açık kalır
    }
}
```

### Asla gizlenmeyenler

Aşağıdakiler otomatik olarak kapsam dışında bırakılır; derleme zamanında sabit
olmak zorunda oldukları için gizlenmeleri derlemeyi bozardı:

- `const val` başlatıcıları, örneğin `const val TAG = "MyScreen"`
- Annotation argümanları, örneğin `@SerialName("user_id")`,
  `@GET("users/{id}")`, `@Deprecated("…")`
- Annotation sınıflarının gövdeleri, yani varsayılan değerleri

StringGuard'ın proje genelinde açılabilmesinin nedeni budur: Retrofit uç
noktaları, `@Serializable` alan adları ve benzeri değerler değişmeden çalışmaya
devam eder.

### Seçerek dahil etme modu

Yalnızca işaretlediğiniz kapsamların gizlenmesini istiyorsanız, tüm metinleri
gizleme davranışını kapatın ve ilgili kapsamları `@Obfuscate` ile işaretleyin:

```kotlin
stringGuard { obfuscateAll = false }
```
```kotlin
import com.stringguard.annotations.Obfuscate

@Obfuscate
object Secrets {
    val apiKey = "sk_live_..."    // gizlenir; projenin geri kalanı açık kalır
}
```

## Yapılandırma başvurusu

```kotlin
stringGuard {
    enabled = true                 // ana anahtar; false = eklenti işlem yapmaz
    obfuscateAll = true            // true (varsayılan): tüm proje; false: yalnızca @Obfuscate kapsamları
    minLength = 1                  // bundan kısa metinler açık kalır
    enabledForDebug = false        // debug build type'larında da gizle (kapalı = hızlı, aranabilir geliştirme derlemeleri)
    licenseKey = "<token>"         // ZORUNLU: lisans metni veya lisans dosyasının yolu

    guard = false                  // @GuardWith reveal noktalarında çalışma zamanı kendini koruma
    tamperBinding = false          // Android: metinler yalnızca şu sertifikalardan biriyle...
    signingCertSha256("AB:CD:...") // ...imzalanmış bir APK içinde çözülür (birden çok kez yazılabilir)

    // AEAD primitifi (varsayılan AES_GCM). ChaCha20-Poly1305 da desteklenir:
    algorithm = StringGuardAlgorithm.CHACHA20_POLY1305
    keepRules {
        keepPackage("com.app.res.*")
        keepMatching("^android\\.")
    }
}
```

| Seçenek | Varsayılan | Etkisi |
|---|---|---|
| `enabled` | `true` | `false` olduğunda eklentinin tamamını kapatır. |
| `obfuscateAll` | `true` | Tüm projeyi gizler veya yalnızca `@Obfuscate` kapsamlarını işler. |
| `minLength` | `1` | Belirtilen karakter sayısından kısa metinleri atlar. |
| `enabledForDebug` | `false` | Varsayılan olarak debug derlemeleri açık bırakılır; geliştirme hızlı, loglar aranabilir kalır. Test derlemeleri her zaman kapsam dışıdır. |
| `licenseKey` | `""` | **Zorunlu.** Çevrimdışı lisans: lisans metni veya onu içeren dosyanın yolu (göreli yollar modül dizinine göre çözülür). Gömülü bir açık anahtarla, ağ bağlantısı olmadan doğrulanır. Eksik, bozuk veya süresi dolmuş anahtar **derlemeyi kırar**. Bkz. [Lisanslama](#lisanslama). |
| `guard` | `false` | `@GuardWith` kapsamlarındaki reveal noktalarına çalışma zamanı kendini koruma dokur. Bkz. [Çalışma zamanı koruması](#çalışma-zamanı-koruması-rasp). |
| `tamperBinding` | `false` | Yalnızca Android. Android metinlerini imzalama sertifikalarınıza bağlar. Bkz. [İmzaya bağlama](#i̇mzaya-bağlama-android). |
| `signingCertSha256(...)` | yok | `tamperBinding` için izin verilen imzalama sertifikalarının SHA-256 parmak izleri. Özellik açıkken zorunludur. |
| `algorithm` | `AES_GCM` | AEAD primitifi: `AES_GCM` (varsayılan) veya `CHACHA20_POLY1305`. Runtime, her hedefte ikisini de çözer. |

### Debug ve release derlemeleri

Test kodu hiçbir zaman gizlenmez. Debug build type'ları, `enabledForDebug = true`
yazılmadığı sürece açık bırakılır; bu, geliştirme döngüsünü hızlı ve logları
aranabilir tutar. Release derlemeleri gizlenir.

<a id="lisanslama"></a>

## Lisanslama

StringGuard'ın tek bir özellik kümesi ve tek bir lisans türü vardır. Geçerli bir
anahtar, bu belgede anlatılan her şeyi her hedefte etkinleştirir: ücretsiz,
deneme veya ara bir katman yoktur ve lisanslı bir derlemeden hiçbir şey
kısılmaz.

Doğrulama çevrimdışıdır. Anahtar, eklentiye gömülü bir açık anahtarla,
yapılandırma anında ve ağ bağlantısı olmadan denetlenir; bu yüzden CI'da ve
ağdan yalıtılmış derlemelerde de çalışır.

**Geçerli lisansı olmayan derleme kırılır.** Eksik, bozuk veya süresi dolmuş bir
anahtar, yapılandırmayı açıklayıcı bir hatayla durdurur:

```
> StringGuard requires a license key. Set stringGuard.licenseKey to your license
  token, or to the path of a file holding it. To build this module without
  StringGuard instead, set stringGuard.enabled = false.
```

Bu bilinçli bir tercihtir. Alternatifi — devam edip daha azını gizlemek — yeşil
bir derlemeyi, içinde okunabilir sırlar taşıyan bir çıktıya dönüştürür; bir metin
gizleyicinin asla üretmemesi gereken sonuç tam olarak budur. Gerçekten
gizlenmemesi gereken bir modül `stringGuard { enabled = false }` ile devre dışı
bırakılır ve o zaman anahtara ihtiyaç duymaz.

Başarılı bir derleme lisansı bir kez günlüğe yazar:

```
StringGuard: license verified (holder=Acme Inc, expires=2027-04-01)
```

<a id="tamper-binding"></a>

## İmzaya bağlama (Android)

İmzaya bağlama, gizlenen her Android metnini uygulamanızın imzalama
sertifikasına bağlar. Listelenen sertifikalardan biriyle imzalanmış bir APK
içinde metinler normal biçimde çözülür. Birisi APK'yı kendi anahtarıyla yeniden
imzalarsa, yani yeniden paketlemenin olağan adımını uygularsa, bağlı her metin
makul görünen anlamsız bir değere çözülür. Uygulama çökmediği için kontrolün
varlığı saldırgana belli edilmez.

```kotlin
stringGuard {
    tamperBinding = true
    signingCertSha256("AB:CD:EF:...")   // Play App Signing anahtarı: Play Console → Test and release → App integrity
    signingCertSha256("12:34:56:...")   // bu anahtarla imzalanan derlemeler çalışacaksa upload veya debug anahtarı
}
```

Parmak izini şu komutlardan biriyle alabilirsiniz:

```bash
apksigner verify --print-certs app-release.apk   # "certificate SHA-256 digest"
keytool -list -v -keystore release.jks           # "SHA256:"
```

- Play App Signing kullanıldığında kullanıcıların kurduğu APK, sizin upload
  anahtarınızla değil Google'ın anahtarıyla imzalanır. Play App Signing
  sertifikasını listeleyin, aksi hâlde üretim kurulumlarında anlamsız değerler
  elde edilir.
- `tamperBinding = true` iken hiç sertifika verilmemişse veya biçimi hatalı bir
  sertifika varsa derleme configuration aşamasında başarısız olur; yanlış
  yapılandırılmış bir uygulama kazara yayımlanamaz.
- Yalnızca Android derlemeleri bağlanır. Aynı projedeki iOS, Desktop, JS ve Wasm
  hedefleri etkilenmez.
- Debug derlemeleri varsayılan olarak gizlenmez. `enabledForDebug = true`
  kullanıyorsanız debug sertifikasını da listeleyin.
- Bir release varyantının JVM birim testleri cihaz dışında, yani APK olmadan
  çalışır. Eklenti, bağlı metinlerin orada okunabilir kalması için listedeki ilk
  sertifikayı Gradle `Test` görevlerine aktarır. Runtime bu istisnayı cihaz
  üzerinde hiçbir zaman kabul etmez.
- Sınırlar: bu önlem, kodu değiştirmeden yapılan yeniden imzalamayı durdurur.
  Kodu yamalayabilen bir saldırgan, özgün APK'nızda zaten herkese açık olan
  sertifika özetini geri döndürebilir.

Sertifika, kurulu APK'nın imza bloğundan (v3, yoksa v2) herkese açık API'ler
üzerinden okunur. `Context` ya da gizli bir Android API'si gerekmez.

<a id="runtime-guard"></a>

## Çalışma zamanı koruması (RASP)

```kotlin
stringGuard { guard = true }   // modüle guard runtime'ını ekler
```
```kotlin
import com.stringguard.annotations.GuardWith

@GuardWith
object PaymentKeys {
    val merchantKey = "mk_live_..."
}
```

Bir `@GuardWith` kapsamındaki korumalı değer, yalnızca süreç temiz görünüyorsa
açılır. Bir hata ayıklayıcı veya kanca altyapısı varsa gerçek değer hiç çözülmez:
aynı uzunlukta, makul görünen bir yanıltıcı değer döndürülür. Böylece uygulama
çalışmaya devam eder ve kontrol, süreci yoklayan kişiye ilan edilmez.

Platform başına neler algılanır:

| Sinyal | Android | JVM / Desktop | iOS | macOS | JS / Wasm |
|---|---|---|---|---|---|
| Hata ayıklayıcı bağlı | ✓ | ✓ | ✓ | ✓ | — |
| Kanca altyapısı | ✓ | ✓ | ✓ | ✓ | — |
| Root / jailbreak | ✓ | ✓ (Linux) | ✓ (yalnızca cihazda) | — | — |

Güvenebileceğiniz davranışlar ve size bıraktığı kararlar:

- **Root'lu veya jailbreak'li cihaz raporlanır ama değerleri bozmaz**; çünkü pek
  çok meşru kullanıcı telefonunu root'lar. Bunun yerine bu cihazları reddetmek
  için başlangıçta `Guard.rootIsSuspicious = true` ayarlayın. Bu, gerçek
  kullanıcıları geri çevirmeye dair bir ürün kararıdır; bilinçli verin.
- **Bir hata ayıklayıcı veya kanca bir kez görüldüğünde**, araç ayrılsa bile
  değerler sürecin ömrü boyunca bozulmuş kalır.
- **Algılamalar size raporlanır, bize değil.** `Detections.onDetection`, her
  sinyal kategorisi ilk kez tetiklendiğinde çağrılır; `Detections.all()` ise o
  ana kadar görülen her şeyi listeler — bunu başlangıçta da okuyun, çünkü bir
  sinyal siz geri çağrıyı kurmadan önce tetiklenebilir. StringGuard hiçbir ağ
  çağrısı yapmaz ve hiçbir şey toplamaz; kaydın nereye gideceğine sizin kendi
  telemetriniz karar verir.

  ```kotlin
  Detections.onDetection = { category ->
      analytics.log("stringguard.detection", mapOf("category" to category.name))
  }
  Detections.all().forEach { /* başlangıçta tetiklenmiş olanlar */ }
  ```

  Olay bir kategori taşır — `DEBUGGER`, `HOOKING_FRAMEWORK`,
  `ROOT_OR_JAILBREAK` — eşleşen dosyayı veya kütüphaneyi asla taşımaz. Onu
  yazmak, dedektörün kendi string'lerini düz metin olarak, saldırganın kontrol
  ettiği bir cihazda okunabilen bir log'a geri koymak olurdu. Geri çağrı,
  sinyali gören thread üzerinde çalışır; bu yüzden hemen dönün. Fırlattığı
  hiçbir şey dışarı sızmaz, bir açma işlemini düşürmesine izin verilmez.

  Root, değerleri bozmadığı durumda bile raporlanır; böylece kaç kullanıcınızın
  rootlu cihazı olduğunu, onları düşman saymadan görebilirsiniz. Herhangi bir
  şeyi zorunlu kılmadan önce toplayın: bir cihazın kanca çerçevesi raporlaması
  az şey ifade eder, tek bir sürümde dört yüz cihazın raporlaması eyleme
  geçilecek bir şeydir.
- **Algılama süreç başınadır.** Bir kontrol yalnızca kendisini yapan süreci
  gözlemler. Uygulamanız sırları başka bir süreçte açıyorsa (`android:process=`,
  yalıtılmış bir servis, ayrı bir native süreç), `@GuardWith` kapsamları ve
  `Guard.report()` çağrısı da o süreçte olmalıdır; ana süreçteki bir kontrol
  diğerleri hakkında hiçbir şey söylemez. `Guard.rootIsSuspicious` da süreç
  başınadır, her birinde ayrıca ayarlayın. Attestation bundan etkilenmez: o,
  cihazdaki uygulamanın tamamını bağlar.
- **Kontroller kısa süre önbelleklenir**; tek ekrandaki pek çok korumalı metin
  tek bir yoklamaya mal olur.
- **Sınırlar.** Her RASP kontrolü, cihaz üzerinde tam denetime sahip kararlı bir
  saldırgan tarafından aşılabilir. Bu, dinamik analizin maliyetini yükseltir;
  bir garanti değildir. Sunucu tarafındaki bir sırrın arkasında derinlemesine
  savunma olarak görün, onun yerine geçen bir şey olarak değil.

### R8 ve küçültme

StringGuard çıktıları kendi consumer ProGuard kurallarını birlikte taşır; bu
nedenle R8 kullanan bir Android uygulamasında elle yapılandırma gerekmez,
kuralları R8 kendiliğinden alır. Gizlenen metinler küçültülmüş bir derlemede de
doğru biçimde çözülür ve R8, public API dışındaki her şeyi yeniden adlandırmakta
serbesttir; yani küçültme dağıtılan dex'i okumayı kolaylaştırmaz, zorlaştırır.

R8 ve StringGuard birbirini tamamlar, birlikte kullanılmak üzere tasarlandı: R8
yeniden adlandırıp küçültür, StringGuard ise R8'in okunabilir bıraktığı metin
içeriklerini şifreler.

## Açık metin kalmadığını doğrulama

Korunan literal'ler derlenmiş çıktıda görünmez ve hiçbir StringGuard sürümü,
bu desteklenen her backend'de gösterilmeden yayınlanmaz. JVM ve Desktop'ta
doğrudan görebilirsiniz:

```bash
./gradlew build
javap -c -p -classpath build/libs/<sizin>.jar com.example.Config
# sır metinleri yok
```

## Sorun giderme

**"Incompatible Gradle JVM version … supports Java 1.8 and 24" (Android
Studio).** Android Studio ile gelen JBR, Gradle 8.14'ün kabul etmediği JDK 25
olabilir. Gradle JDK ayarını 17 ile 24 arasında bir sürüme yönlendirin:
`Settings → Build, Execution, Deployment → Build Tools → Gradle → Gradle JDK`.
Alternatif olarak `gradle.properties` dosyasına
`org.gradle.java.home=/path/to/jdk21` satırını ekleyin ve ardından
*Sync Project with Gradle Files* işlemini çalıştırın.

**"Plugin `com.stringguard` not found."** Lisansınızın bildirdiği depo
`pluginManagement.repositories` içinde yok ya da kimlik bilgileri
yapılandırılmamış. Bkz. [Adım 1](#adım-1-depo-yapılandırması).

**"Could not find com.stringguard:runtime" / ":annotations".** Aynı depo
`dependencyResolutionManagement.repositories` içinde yok. Her iki blok da ona
ihtiyaç duyar.

**Şifrelenmesini beklediğim bir metin hâlâ açık görünüyor.** Büyük olasılıkla bir
`const val`, bir annotation argümanı, `minLength` sınırının altında bir değer,
`@DontObfuscate`/`@KeepPlain` kapsamındaki bir metin, bir `keepRules` kalıbıyla
eşleşen bir değer veya `enabledForDebug = false` ile alınmış bir debug
derlemesidir.

**Retrofit / kotlinx.serialization annotation değerleri değişti mi?**
Değişmez. Annotation argümanları tasarım gereği hiçbir zaman gizlenmez.

**`tamperBinding` açıkken kurulu uygulamada metinler bozuk görünüyor.** Kurulu
APK, listelemediğiniz bir sertifikayla imzalanmıştır. Play App Signing
kullanıyorsanız bu Google'ın anahtarıdır: parmak izini Play Console → App
integrity bölümünden ekleyin. APK'yı `apksigner verify --print-certs` ile
kontrol edin.

**"tamperBinding = true requires at least one signingCertSha256".** Parmak
izlerini [İmzaya bağlama](#i̇mzaya-bağlama-android) bölümünde gösterildiği gibi ekleyin.

**"StringGuard requires a license key."** `stringGuard.licenseKey` ayarlanmamış
veya boş bir dosyaya işaret ediyor. Lisans anahtarınızı ya da onu içeren bir
dosyanın yolunu verin (göreli yollar modül dizinine göre çözülür). Hiç
gizlenmemesi gereken bir modül için bunun yerine
`stringGuard { enabled = false }` kullanın; o zaman anahtar gerekmez.

**"StringGuard license is not valid: ..."** Anahtar bir StringGuard lisans
token'ı değil, kesilmiş ya da StringGuard'ın bu sürümü için kesilmemiş.
Mesajdaki sebep hangisi olduğunu söyler. `license format v1 is no longer
supported` sebebi, anahtarın artık kullanılmayan bir biçimde olduğunu gösterir;
yeniden kesilmesini isteyin.

**"StringGuard license '...' expired on ...".** Lisansı yenileyip
`stringGuard.licenseKey`'i güncelleyin. Derlemelerin korumasız sürmesine izin
verilmediği için bir atlatma yolu yoktur.

## Attestation (isteğe bağlı, ileri düzey)

Attestation, bir kurulumun sağlam bir cihaz üzerinde gerçek ve lisanslı bir
kurulum olup olmadığını yanıtlayan, sunucu eşgüdümlü bir katmandır. Yalnızca
istemcide çalışan RASP'tan daha güçlüdür, çünkü karar; uygulamanın üretemeyeceği
imzalı materyale dayanarak sunucu tarafında verilir.

- **Platformun izin verdiği yerde donanım köklü.** Android'de cihazın
  donanım destekli anahtar attestation'ı, iOS'ta Apple App Attest kullanılır.
  Hiçbiri Play Integrity, Google API'si veya istek kotası gerektirmez ve
  doğrulama çevrimdışıdır.
- **İstek bağlama.** Hassas istekler attestation'lı cihaz anahtarıyla imzalanır;
  bu yüzden ele geçirilip yeniden kullanılan bir oturum belirteci, ait olduğu
  cihaz olmadan işe yaramaz.
- **Varsayılan olarak fail-open.** Attestation bir oturumun güvence düzeyini
  etiketler; dağıtılan varsayılan "izin ver ve raporla" biçimindedir, toptan
  engelleme değil. Düşük bir güvence düzeyinin işlem başına neye mal olacağına
  siz karar verirsiniz.
- **Kendi altyapınızda çalışır.** Doğrulama servisi sizin altyapınızda koşar;
  cihaz trafiği oradan hiç çıkmaz. Açıp çalıştırdığınız bir paket olarak
  teslim edilir; yayına almadan önce çalıştıracağınız bir yapılandırma
  denetimi, lisans tanımlama, cihaz yönetimi ve iptal için de bir panel ve
  bir API ile birlikte gelir.
- iOS App Attest, uygulamanın provisioning profile'ında App Attest yeteneğini ve
  fiziksel bir cihaz gerektirir.

Neyi kanıtladığı, neyi kanıtlamadığı, nasıl entegre edilip işletileceği için
[docs/ATTESTATION.md](docs/ATTESTATION.md) belgesine bakın.

## Lisans

StringGuard ticari ve tescilli bir yazılımdır. Açık kaynak değildir ve kaynak
kodu için herhangi bir lisans verilmez.

StringGuard ile derleme yapmak ticari bir lisans anahtarı gerektirir. Tek bir
özellik kümesi vardır, ücretsiz katman yoktur. Geçerli bir anahtarla ticari
kullanım dâhil her amaçla derleme yapabilir, çalışma zamanı bileşenlerini kendi
uygulamanıza gömüp o uygulamayı uygulama mağazaları üzerinden ve
müşterilerinize dağıtabilirsiniz. Uygulamanızın daha önce dağıttığınız bir
sürümüne gömülmüş çalışma zamanı bileşenleri, anahtarınızın süresi dolduktan
sonra da son kullanıcılarınız için lisanslı kalır. Artifact'lar — örneğin paket
kayıtları ve aynaları tarafından — değiştirilmeden yeniden dağıtılabilir.

Şartların tamamı [LICENSE](LICENSE) dosyasındadır. Üçüncü taraf bileşenler ve
lisansları [NOTICE](NOTICE) dosyasında listelenmiştir.
