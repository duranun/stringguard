# Entegrasyon Dokümanı

[English](INTEGRATION.md) · **Türkçe**

Bu doküman StringGuard'ı bir projeye eklemek, yapılandırmak ve sonucu doğrulamak için gereken adımları açıklar.

## Gereksinimler

StringGuard, JDK 17 ile 24 arasındaki sürümlerle, Gradle 8.x ve Kotlin 2.2.0 ile çalışır. Android projelerinde Android Gradle Plugin 8.x kullanılması beklenir.

Eklenti belirli bir Kotlin sürümüne göre derlendiğinden, projenizin Kotlin sürümünün yukarıdaki sürümle eşleşmesi gerekir.

## Kurulum

StringGuard'ın yayınlandığı Maven deposunu `settings.gradle.kts` dosyanızdaki her iki repository bloğuna da ekleyin. İlk blok eklentinin kendisini, ikinci blok ise eklentinin eklediği `annotations` ve `runtime` bağımlılıklarını çözmek için gereklidir.

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

Depo herkese açık olduğundan herhangi bir kimlik bilgisi gerekmez.

## Eklentinin uygulanması

Eklentiyi, modülün kullandığı Kotlin eklentisinin yanına ekleyin. `annotations` ve `runtime` bağımlılıkları ilgili yapılandırmaya otomatik olarak eklenir.

Android projelerinde:

```kotlin
plugins {
    id("com.android.application")
    kotlin("android")
    id("com.stringguard") version "0.1.0"
}

stringGuard {
    enabled = true
    minLength = 4
}
```

JVM veya Desktop projelerinde:

```kotlin
plugins {
    kotlin("jvm") version "2.2.0"
    id("com.stringguard") version "0.1.0"
}

stringGuard {
    enabled = true
    minLength = 4
}
```

Kotlin Multiplatform ve Compose Multiplatform projelerinde:

```kotlin
plugins {
    kotlin("multiplatform") version "2.2.0"
    id("com.stringguard") version "0.1.0"
}

stringGuard {
    enabled = true
    minLength = 4
}
```

Multiplatform projelerinde bağımlılıklar `commonMain` kaynak setine eklenir ve belgelenen tüm hedefler aynı yapılandırmayla kapsanır.

## Hariç tutma kuralları

StringGuard varsayılan olarak uygun tüm string'leri dönüştürür. Dönüştürülmemesi gereken değerleri bu kapsamın dışında bırakabilirsiniz.

Bir dosyanın, sınıfın veya nesnenin dönüştürülmeden kalmasını istiyorsanız `@DontObfuscate` ek açıklamasını kullanın:

```kotlin
import com.stringguard.annotations.DontObfuscate

@DontObfuscate
object PublicConstants {
    val featureFlag = "enable_new_home"
}
```

Değerleri desenlere göre hariç tutmak için `keepRules` bloğunu kullanabilirsiniz:

```kotlin
stringGuard {
    minLength = 4
    keepRules {
        keepPackage("com.myapp.resources.*")
        keepMatching("^https://cdn\\.")
    }
}
```

Derleme zamanı sabitleri ve ek açıklama argümanları, dilin bunları sabit değer olarak gerektirmesi nedeniyle hiçbir koşulda dönüştürülmez.

Tüm projeyi dönüştürmek yerine yalnızca belirli yerleri dönüştürmek isterseniz `obfuscateAll` seçeneğini kapatıp ilgili bölümleri `@Obfuscate` ile işaretleyebilirsiniz:

```kotlin
stringGuard { obfuscateAll = false }
```

```kotlin
import com.stringguard.annotations.Obfuscate

@Obfuscate
class PaymentClient {
    private val merchantSecret = "msk_live_..."
}
```

## Yapılandırma seçenekleri

| Seçenek | Varsayılan | Açıklama |
|---|---|---|
| `enabled` | `true` | Eklentiyi etkinleştirir veya devre dışı bırakır. |
| `obfuscateAll` | `true` | Tüm projenin mi yoksa yalnızca `@Obfuscate` ile işaretlenen yerlerin mi dönüştürüleceğini belirler. |
| `minLength` | `1` | Bu uzunluğun altındaki string'ler dönüştürülmez. |
| `enabledForDebug` | `false` | Debug derlemelerini de dönüştürür. Test kaynakları her durumda kapsam dışıdır. |
| `licenseKey` | `""` | Çevrimdışı lisans anahtarı. Boş veya geçersiz olduğunda ücretsiz katman geçerli olur. |

## Doğrulama

JVM veya Desktop çıktısında string'lerin dönüştürülüp dönüştürülmediğini `javap` ile inceleyebilirsiniz:

```bash
./gradlew build
javap -c -p -classpath build/libs/<jar>.jar com.example.Config
```

Android'de APK içindeki `dex` dosyalarını tarayabilirsiniz:

```bash
./gradlew assembleRelease
unzip -o app/build/outputs/apk/release/*-release*.apk -d /tmp/apk >/dev/null
strings /tmp/apk/classes*.dex | grep -F "beklenen_deger"
```

Kotlin/Native çıktısında string'ler UTF-16LE olarak saklandığından her iki kodlamayı da taramanız gerekir:

```bash
strings -e l <binary> | grep -F beklenen_deger
strings       <binary> | grep -F beklenen_deger
```

## Sorun giderme

Android Studio "Incompatible Gradle JVM version" uyarısı veriyorsa, Gradle JDK ayarını 17 ile 24 arasında bir sürüme getirin. Bu ayara Settings > Build, Execution, Deployment > Build Tools > Gradle yolundan ulaşabilir ya da `gradle.properties` dosyasına `org.gradle.java.home` değerini ekleyebilirsiniz.

`Plugin com.stringguard was not found` hatası, Maven deposunun `pluginManagement` bloğuna eklenmediğini gösterir.

`Could not find com.stringguard:runtime` veya benzeri bir bağımlılık hatası, deponun `dependencyResolutionManagement` bloğuna da eklenmesi gerektiği anlamına gelir.
