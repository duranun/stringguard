# StringGuard — Entegrasyon Kılavuzu

[English](INTEGRATION.md) · **Türkçe**

StringGuard'ı gerçek bir projeye eklemek için adım adım kılavuz. Kısa sürüm için
[README](../README.tr.md)'ye bak; bu tam anlatımdır.

**İçindekiler**
1. [Gereksinimler](#1-gereksinimler)
2. [Kurulum](#2-kurulum)
3. [Modülüne uygula](#3-modülüne-uygula)
4. [İlk build & sağlık kontrolü](#4-i̇lk-build--sağlık-kontrolü)
5. [Ne obfuscate edileceğini ayarlama](#5-ne-obfuscate-edileceğini-ayarlama)
6. [Debug, release & CI](#6-debug-release--ci)
7. [Binary'yi doğrulama](#7-binaryyi-doğrulama)
8. [Mevcut kod tabanını taşıma](#8-mevcut-kod-tabanını-taşıma)
9. [Sorun Giderme](#9-sorun-giderme)
10. [SSS](#10-sss)

---

## 1. Gereksinimler

| Araç | Sürüm |
|---|---|
| JDK | 17–24 (21 önerilir). **25/26 değil** — Gradle 8.14 Java > 24'ü reddeder. |
| Gradle | 8.x (8.14 ile test edildi) |
| Kotlin | **2.2.0** — eklentinin compiler build'iyle eşleşmeli |
| AGP | 8.x (Android) |

Compiler plugin Kotlin sürümüne bağlıdır. Projen Kotlin 2.2.0'da değilse önce onu hizala.

**Zihin modeli:** obfuscate-all **varsayılan açık**. Literal'leri dahil etmezsin; *hariç*
tutarsın. Derleme-zamanı sabit olmak zorunda olan literal'ler (`const val`, annotation
argümanları) asla dokunulmaz, bu yüzden proje genelinde açmak güvenlidir.

---

## 2. Kurulum

StringGuard Maven deposunu `settings.gradle.kts`'in **her iki** repository bloğuna ekle —
`pluginManagement` eklentiyi, `dependencyResolutionManagement` ise eklentinin senin için
eklediği `annotations` + `runtime` kütüphanelerini çözer:

```kotlin
pluginManagement {
    repositories {
        maven("https://duranun.github.io/stringguard/maven")
        gradlePluginPortal()
        google()
        mavenCentral()
    }
}
dependencyResolutionManagement {
    repositories {
        maven("https://duranun.github.io/stringguard/maven")
        google()
        mavenCentral()
    }
}
```

Kimlik bilgisi gerekmez — depo public ve salt-okunur.

---

## 3. Modülüne uygula

StringGuard'ı modülün zaten kullandığı Kotlin plugin'inin yanına uygula. **Sıra önemli
değildir.** `annotations`/`runtime`'ı elle **eklemezsin**; eklenti doğru
configuration/source set'e ekler.

### 3.1 Düz Android (klasik, KMP olmayan)

```kotlin
// app/build.gradle.kts
plugins {
    id("com.android.application")      // veya com.android.library
    kotlin("android")
    id("com.stringguard") version "0.1.0"
}

stringGuard {
    enabled = true
    minLength = 4
}
```
> Bağımlılıklar `implementation` configuration'ına eklenir.

### 3.2 JVM / Desktop

```kotlin
plugins {
    kotlin("jvm") version "2.2.0"
    id("com.stringguard") version "0.1.0"
}

stringGuard { enabled = true; minLength = 4 }
```

### 3.3 Kotlin Multiplatform / Compose Multiplatform

```kotlin
plugins {
    kotlin("multiplatform") version "2.2.0"
    id("com.stringguard") version "0.1.0"
}

stringGuard { enabled = true; minLength = 4 }
```
`annotations` + `runtime` bağımlılıkları `commonMain`'e eklenir; böylece **her** hedef
(Android, iOS/Native, JS, Wasm, Desktop) aynı yapılandırmayla kapsanır.

---

## 4. İlk build & sağlık kontrolü

```bash
./gradlew assembleRelease   # Android
./gradlew build             # JVM / KMP
```

Kodun değişmez — sıradan string literal'leri yazarsın:

```kotlin
object Config {
    val apiBaseUrl = "https://api.example.com/v2"   // release binary'de obfuscate edilir
    val analyticsKey = "ak_prod_1a2b3c4d5e"         // obfuscate edilir
}
```

Çalışma anında değerler yazdığınla aynıdır; binary'de yok olurlar.
[7. bölümle](#7-binaryyi-doğrulama) doğrula.

---

## 5. Ne obfuscate edileceğini ayarlama

### Bir scope'u okunur bırak — `@DontObfuscate`

```kotlin
import com.stringguard.annotations.DontObfuscate

@DontObfuscate                        // dosya, class, object, function, property veya constructor
object ReflectionKeys {
    val lookupName = "com.example.Thing"   // Class.forName ile kullanılır → okunur kalsın
}
```
`@KeepPlain` birebir eşanlamlıdır — aynı davranış, hangi isim daha iyi okunuyorsa onu seç.

### Desen tabanlı hariç tutma — `keepRules`

```kotlin
stringGuard {
    minLength = 4
    keepRules {
        keepPackage("com.example.resources.*")   // paket + tüm alt paketleri düz kalır
        keepMatching("^https://cdn\\.")           // regex'e uyan literal'ler düz kalır
    }
}
```

- `keepPackage("a.b.*")` hem `a.b` paketini **hem de** her alt paketini korur.
- `keepMatching(regex)` içeriği eşleşen her literal'i korur (`containsMatchIn` kullanır).

### Otomatik olarak asla obfuscate edilmeyenler

Derleme-zamanı sabit olmak *zorunda* oldukları için otomatik atlanır; obfuscate edilseler
derlenmez:

- `const val` initializer'ları
- Annotation argümanları (`@SerialName("…")`, `@GET("…")`, `@Deprecated("…")`, …)
- Annotation class default değerleri

Retrofit, kotlinx.serialization ve Room'un obfuscate-all açıkken çalışmaya devam etmesinin
nedeni budur.

### Opt-in modu (hariç tutmak yerine işaretle)

```kotlin
stringGuard { obfuscateAll = false }
```
```kotlin
import com.stringguard.annotations.Obfuscate

@Obfuscate
class PaymentClient {
    private val merchantSecret = "msk_live_..."   // obfuscate edilir; diğer her şey düz kalır
}
```

---

## 6. Debug, release & CI

- **Test** derlemeleri **her zaman** atlanır.
- **Debug** build türleri, `enabledForDebug = true` olmadıkça düz bırakılır.
- **Release** obfuscate edilir.

```kotlin
stringGuard { enabledForDebug = false }   // dev build'leri hızlı, loglar greplenebilir kalsın
```

**Tam DSL:**

| Seçenek | Varsayılan | Etki |
|---|---|---|
| `enabled` | `true` | false iken tüm eklenti kapanır. |
| `obfuscateAll` | `true` | Tüm proje mi yoksa yalnızca `@Obfuscate` scope'ları mı. |
| `minLength` | `1` | N karakterden kısa literal'leri atlar. |
| `enabledForDebug` | `false` | Debug build türlerini de obfuscate et. Test kodu her zaman atlanır. |
| `licenseKey` | `""` | Offline lisans anahtarı/yolu. Geçersiz/boş → free tier; **build asla engellenmez**. |
| `algorithm` | `AES_GCM` | *Rezerve* — bugün her zaman AES-GCM kullanılır. |
| `tamperBinding` | `false` | *Rezerve (yol haritası)* — imzaya bağlı anahtar. |

CI özel bir şey gerektirmez — normal release build'ini çalıştır. CI JDK 25/26 image'ı
kullanıyorsa 17–24 arası JDK sabitle. Aşağıdaki düz-metin-sızıntı kontrolünü build geçidi
olarak eklemeyi düşün.

---

## 7. Binary'yi doğrulama

**JVM / Desktop:**
```bash
./gradlew build
javap -c -p -classpath build/libs/<senin>.jar com.example.Config
# sır literal'leri gitti; çağrılar stringGuardReveal([B) üzerinden gider
```

**Android:**
```bash
./gradlew assembleRelease
unzip -o app/build/outputs/apk/release/*-release*.apk -d /tmp/apk >/dev/null
strings /tmp/apk/classes*.dex | grep -F "ak_prod_1a2b3c4d5e" && echo "SIZINTI!" || echo "temiz"
```

**Kotlin/Native (iOS backend):** Kotlin/Native string'leri **UTF-16LE** saklar; iki
kodlamayı da tara, yoksa yanlış "temiz" alırsın:
```bash
strings -e l <binary> | grep -F sirin   # UTF-16LE (önemli olan)
strings       <binary> | grep -F sirin   # UTF-8
```

---

## 8. Mevcut kod tabanını taşıma

1. **Eklentiyi uygula** ([3. bölüm](#3-modülüne-uygula)) ve bir kez build al. Obfuscate-all
   varsayılan olduğu için uygun her şey hemen obfuscate edilir — kod değişikliği yok.
2. **Release build al ve testlerini / smoke test'lerini çalıştır.** Korkuluklar `const val`
   ve annotation argümanlarını kapsar; uygulaman bir *runtime* string literal'ini bir yerde
   kimliğine göre okuyorsa (bir literal üzerinde reflection, bir literal ile anahtarlanan elle
   yazılmış resource lookup), onu `@DontObfuscate` veya bir `keepRules` girdisiyle hariç tut.
3. **`minLength`'i yükselt** (ör. `4`) — önemsiz literal'leri atla, çıktıyı küçült.
4. **Düz-metin-sızıntı kontrolünü** ([7. bölüm](#7-binaryyi-doğrulama)) CI'a ekle.
5. **Opt-in** modeli mi tercih ediyorsun? `obfuscateAll = false` yap ve hassas scope'ları
   `@Obfuscate` ile işaretle.

**Güvenli, işlem gerektirmeyen:** Retrofit/Ktor endpoint annotation'ları, `@SerialName` alan
adları, `const val` log tag'leri, annotation argümanı kullanan DI qualifier'ları.

---

## 9. Sorun Giderme

**`Incompatible Gradle JVM version … supports Java 1.8 and 24` (Android Studio).**
Android Studio'nun gömülü JBR'si JDK 25 olabilir; Gradle 8.14 reddeder. Gradle JDK'sını
17–24 arasına ayarla: *Settings → Build, Execution, Deployment → Build Tools → Gradle →
Gradle JDK*, ya da `gradle.properties` içinde `org.gradle.java.home=/jdk21/yolu` tanımla,
sonra *Sync Project with Gradle Files*.

**`Plugin [id: 'com.stringguard'] was not found`.** StringGuard Maven deposu
`pluginManagement.repositories`'de eksik (veya URL yanlış). Bkz. [2. bölüm](#2-kurulum).

**`Could not find com.stringguard:runtime` / `:annotations`.** StringGuard Maven deposunu
`dependencyResolutionManagement.repositories`'ye de ekle.

**Obfuscate olmasını beklediğim literal hâlâ düz metin.** Kontrol et: bir `const val` mi;
annotation argümanı mı; `minLength`'ten kısa mı; `@DontObfuscate`/`@KeepPlain` scope'unda mı;
bir `keepRules` desenine mi uyuyor; `enabledForDebug = false` ile debug build'inde mi.

**Çalışma anında okunur lazım olan bir string (reflection) obfuscate oldu.** O scope'u
`@DontObfuscate` veya bir `keepRules` girdisiyle hariç tut.

---

## 10. SSS

**Bir şeyi annotate etmek zorunda mıyım?** Hayır. Obfuscate-all varsayılan; annotation'lar
yalnızca hariç tutmak (`@DontObfuscate`/`@KeepPlain`) veya opsiyonel opt-in modu
(`@Obfuscate`) için.

**R8/ProGuard olmadan çalışır mı?** Evet, ve onlarla birlikte — tamamlayıcıdırlar (R8
adlandırır/küçültür, StringGuard string içeriğini şifreler).

**Uygulamayı yavaşlatır mı?** Şifre çözme literal başına, küçük ve senkron; tipik sır/config
string'leri için ihmal edilebilir. Devasa veya sıcak-döngü literal'lerini `minLength`/`keepRules`
ile atla.

**Bu gerçek güvenlik mi?** Statik analizin maliyetini yükseltir; sıradan `strings`/decompile
ve otomatik tarayıcıları etkisiz kılar. Kriptografik gizlilik **değildir** — anahtar cihazda
türetilebilir. Asla sızmaması gereken bir sır sunucuya aittir.

**iOS ve Wasm de?** Evet — runtime `commonMain`'de saf-Kotlin; aynı şifreli payload JVM,
Kotlin/Native (iOS backend), JS ve Wasm'da aynı şekilde çözülür.

**Şifreyi ChaCha20'ye değiştirebilir / anahtarı imzaya bağlayabilir miyim?** Henüz hayır —
`algorithm` ve `tamperBinding` DSL tarafından kabul edilir ama rezervedir; bugün her zaman
AES-GCM kullanılır.
