#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h> // Dosya boyutu ve izinlerini okumak için ekledik

// Bir dosyanın sadece ASCII karakterlerden oluşup oluşmadığını kontrol eden fonksiyon
int ascii_kontrol(const char *dosya_adi) {
    FILE *f = fopen(dosya_adi, "r");
    if (f == NULL) return 0;

    int ch;
    while ((ch = fgetc(f)) != EOF) {
        if (!isascii(ch)) {
            fclose(f);
            return 0; 
        }
    }
    fclose(f);
    return 1; 
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Hata: Eksik parametre girdiniz!\n");
        printf("Kullanım:\n  Birleştirme: tarsau -b dosya1 dosya2 -o cikis.sau\n  Açma: tarsau -a arsiv.sau hedef_dizin\n");
        return 1;
    }

    // 1. MOD: Dosyaları Birleştirme (-b)
    if (strcmp(argv[1], "-b") == 0) {
        printf("Birleştirme modu (-b) seçildi.\n");
        
        int dosya_sayisi = 0;
        int o_indeksi = -1;
        char cikis_dosya_adi[256] = "a.sau"; // Varsayılan çıktı adı: a.sau

        // Girdi dosyalarının sınırını belirlemek ve -o parametresini aramak için döngü
        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "-o") == 0) {
                o_indeksi = i;
                // Eğer -o'dan sonra bir dosya adı girilmişse onu çıktı adı yapalım
                if (i + 1 < argc) {
                    strcpy(cikis_dosya_adi, argv[i + 1]);
                }
                break; 
            }
            dosya_sayisi++;
        }

        if (dosya_sayisi > 32) {
            printf("Hata: Giriş dosyası sayısı en fazla 32 olabilir!\n");
            return 1;
        }

        printf("Toplam %d adet girdi dosyası tespit edildi.\n", dosya_sayisi);

        // ASCII Format Kontrolü
        for (int i = 2; i < 2 + dosya_sayisi; i++) {
            if (!ascii_kontrol(argv[i])) {
                printf("%s giriş dosyasının formatı uyumsuzdur!\n", argv[i]);
                return 0; 
            }
        }
        printf("Tüm girdi dosyalarının formatı uygun (ASCII).\n");

        // --- YENİ ADIM: Boyut Kontrolü ve Organizasyon Bilgisi Toplama ---
        long long toplam_boyut = 0;
        char organizasyon_buffer[4096] = ""; // Dosya bilgilerini toplayacağımız string

        for (int i = 2; i < 2 + dosya_sayisi; i++) {
            struct stat st;
            if (stat(argv[i], &st) != 0) {
                printf("Hata: %s dosyasının bilgileri okunamadı!\n", argv[i]);
                return 1;
            }

            toplam_boyut += st.st_size; // Dosya boyutunu toplama ekle
            
            // Linux izin maskesini octal (örneğin 0644) formatta almak için st.st_mode & 0777 kullanırız
            int izinler = st.st_mode & 0777;

            // Bilgileri hoca amcanın istediği formata getiriyoruz: |Dosya adı, izinler, boyut|
            char gecici_kayit[512];
            sprintf(gecici_kayit, "|%s,%o,%ld|", argv[i], izinler, st.st_size);
            strcat(organizasyon_buffer, gecici_kayit);
        }

        // 200 MB sınırı kontrolü (200 * 1024 * 1024 bayt)
        if (toplam_boyut > 200 * 1024 * 1024) {
            printf("Hata: Giriş dosyalarının toplam boyutu 200 MB'ı geçemez!\n");
            return 1;
        }

        printf("Toplam Boyut: %lld bayt (Limit uygun).\n", toplam_boyut);
        printf("Çıktı Dosyası Adı: %s\n", cikis_dosya_adi);
        printf("Oluşturulacak Organizasyon Bölümü:\n%s\n", organizasyon_buffer);
        
        // Bir sonraki adımda bu bilgileri kullanarak gerçek .sau dosyasını yazacağız!
    } 
    
    // 2. MOD: Arşivi Geri Açma (-a)
    else if (strcmp(argv[1], "-a") == 0) {
        printf("Arşiv açma modu (-a) seçildi.\n");
        if (argc > 4) {
            printf("Hata: -a parametresinden sonra en fazla 2 parametre girilebilir!\n");
            return 1;
        }
    } 
    
    else {
        printf("Hata: Geçersiz parametre '%s'! Lütfen -b veya -a kullanın.\n", argv[1]);
        return 1;
    }

    return 0;
}