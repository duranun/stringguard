# StringGuard

[English](README.md) · **Türkçe**

**Kotlin uygulamaları için string obfuscation.**

StringGuard, derleme sırasında string sabitlerini dönüştürerek derlenmiş uygulamalardan bu değerlerin statik analiz yoluyla çıkarılmasını zorlaştıran bir Gradle eklentisidir.

Android uygulamalarındaki bazı string'ler düşündüğümüzden daha fazla bilgi açığa çıkarabilir. API adresleri, dahili tanımlayıcılar, hata mesajları ve uygulamanın çalışma mantığına ilişkin bazı detaylar APK incelenerek kolayca bulunabilir.

StringGuard, Kotlin kodunu yazma şeklinizi değiştirmeden bu tür incelemeleri zorlaştırmayı amaçlar.

## Nasıl çalışır?

StringGuard, derleme sürecinde uygun string sabitlerini dönüştürür. Orijinal değerleri derlenmiş kodda doğrudan bırakmak yerine, çalışma zamanında çözülen farklı bir temsil kullanır.

Böylece kaynak kodda normal Kotlin string'leriyle çalışmaya devam ederken derlenmiş uygulamadaki string'lerin statik olarak çıkarılmasını zorlaştırabilirsiniz.

## Özellikler

- Derleme zamanında string obfuscation
- Yapılandırılabilir hariç tutma kuralları
- Dönüştürülmeden kalması gereken değerler için destek
- Gradle üzerinden entegrasyon
- Belgelenen Kotlin hedefleri için destek

## Kurulum

Kurulum ve yapılandırma adımları için [entegrasyon dokümanına](docs/INTEGRATION.tr.md) göz atın.

## Kullanım

StringGuard, yapılandırıldıktan sonra derleme sürecinin bir parçası olarak çalışır. Kotlin kodunuzu her zamanki gibi yazabilirsiniz:

```kotlin
private const val DEFAULT_TIMEOUT = 30

private val errorMessage = "Authentication failed"
```

Derleme zamanı sabitleri ve desteklenmeyen ya da hariç tutulan değerler dönüştürülmeyebilir. Ayrıntılar için yapılandırma kurallarını inceleyin.

## Sınırlamalar

String obfuscation, uygulamayı bütünüyle şifrelemez ve string'lerin hiçbir koşulda geri elde edilemeyeceği anlamına gelmez.

Uygulamanın çalışma sırasında ihtiyaç duyduğu bir değer, bir noktada uygulama tarafından erişilebilir olmalıdır. Yeterli erişime sahip bir saldırgan, çalışma zamanında bu değerleri yine elde edebilir.

StringGuard'ın amacı statik analizi zorlaştırmaktır. Uygulama güvenliğini destekleyen ek bir katman olarak kullanılmalı; uygulamaya gömülü gizli anahtarları korumanın tek yöntemi olarak görülmemelidir.

## Uyumluluk

Desteklenen Kotlin, Gradle, Android Gradle Plugin ve platform sürümleri için proje dokümantasyonuna bakın.

## Lisans

StringGuard tescilli bir yazılımdır. Eklentiyi kendi uygulamalarınızı geliştirmek ve dağıtmak için kullanabilirsiniz; eklentinin kendisi yeniden dağıtılamaz, satılamaz veya tersine mühendisliğe tabi tutulamaz. Ayrıntılar için [LICENSE](LICENSE) dosyasına bakın.
