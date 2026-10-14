# StringGuard

[English](README.md) · **Türkçe**

Kotlin için derleme-zamanı **string gizleme (obfuscation)** — tek bir Gradle
eklentisiyle **KMP / Compose Multiplatform ve düz (KMP olmayan) Android / JVM**.

Projendeki uygun her string literal derleme anında şifrelenir ve çalışma anında çözen
bir çağrıyla değiştirilir. Düz metin derlenmiş binary'de hiç görünmez; APK / jar /
framework üzerinde `strings` taraması veya sıradan decompile hiçbir şey bulamaz.

```kotlin
val apiKey = "sk_live_51H...Z"   // sen bunu yazarsın
// binary'de: stringGuardReveal(byteArrayOf(0x01, 0x4e, ...))  ← düz metin yok
```

- **Tek eklenti, her Kotlin hedefi** — JVM/Desktop, Android (KMP *ve* klasik), iOS/Native, JS, Wasm.
- **Başlamak için sıfır annotation** — obfuscate-all varsayılan açık. Kodu dahil etmez, *hariç* tutarsın.
- **Yapısı gereği güvenli** — `const val` ve annotation argümanlarına asla dokunulmaz; Retrofit / kotlinx.serialization / Room çalışmaya devam eder.

> **Son sürüm:** `0.1.0` — bkz. [Releases](../../releases).

---

## Kurulum

**1.** `settings.gradle.kts`'e StringGuard Maven deposunu ekle:

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

**2.** Eklentiyi modülüne uygula (Kotlin plugin'inin yanına — sıra önemli değil):

```kotlin
plugins {
    kotlin("android")    // veya kotlin("jvm") / kotlin("multiplatform")
    id("com.stringguard") version "0.1.0"
}
```

**3.** Build al. Hepsi bu — `annotations` + `runtime` bağımlılıkları senin için eklenir
ve obfuscate-all varsayılan açık olduğu için hiçbir şeyi annotate etmene gerek yok.

➡️ **Tam anlatım:** [Entegrasyon Kılavuzu](docs/INTEGRATION.tr.md) · [Integration Guide (EN)](docs/INTEGRATION.md)

---

## Ne obfuscate edileceğini kontrol et

```kotlin
import com.stringguard.annotations.DontObfuscate

@DontObfuscate                 // bir dosya/class/object/function'ı okunur bırak
object PublicConstants {
    val featureFlag = "enable_new_home"
}
```
```kotlin
stringGuard {
    minLength = 4
    keepRules {
        keepPackage("com.myapp.resources.*")   // paket (+ alt paketleri) düz kalır
        keepMatching("^https://cdn\\.")         // bu regex'e uyan literal'ler düz kalır
    }
}
```

`const val` initializer'ları, annotation argümanları (`@SerialName`, Retrofit `@GET`, …)
ve annotation-class gövdeleri **her zaman** düz bırakılır — proje genelinde açmanın
güvenli olmasının nedeni budur. Tersini mi istersin? `stringGuard { obfuscateAll = false }`
yap ve scope'ları `@Obfuscate` ile işaretle.

---

## Dürüst güvenlik modeli

Cihaz üstü obfuscation **kriptografik gizlilik değildir**. Şifre çözme cihazda olur,
anahtar orada türetilebilir; dinamik analiz (debugger, Frida, hook) kullanan kararlı bir
saldırgan değerleri yine çıkarabilir. StringGuard'ın sağladığı şey **statik analizin
maliyetini yükseltmek** — `strings`, sıradan decompile ve otomatik sır tarayıcılarını
etkisiz kılar; sızıntıların çoğu bunlara dayanır. Bir caydırıcı ve IP-koruma katmanı
olarak gör — **asla sızmaması gereken** bir sır için kasa olarak değil; gerçekten gizli
bir değer istemci binary'sine değil sunucuya aittir.

---

## Lisanslama

StringGuard lisansı build anında **offline** doğrular (ağ çağrısı yok, hiçbir şey dışarı
gitmez) ve **build'i asla engellemez** — boş/geçersiz lisans sadece free tier'a düşer.
Premium katmanlar (tüm KMP hedefleri, RASP, gelişmiş sertleştirme) yol haritasında.
Lisans için bir issue aç veya iletişime geç.

---

## Bağlantılar

- 📘 [Entegrasyon Kılavuzu](docs/INTEGRATION.tr.md) (TR) · [Integration Guide](docs/INTEGRATION.md) (EN)
- 🏷️ [Releases](../../releases)
- 🔧 Maven repo: `https://duranun.github.io/stringguard/maven`

Kaynak kod private. Bu sayfa derlenmiş eklentiyi ve dokümantasyonunu dağıtır.
