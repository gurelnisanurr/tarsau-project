#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

// Bir dosyanın sadece ASCII karakterlerden oluşup oluşmadığını kontrol eden fonksiyon
int ascii_kontrol(const char *dosya_adi)
{
    FILE *f = fopen(dosya_adi, "r");
    if (f == NULL)
        return 0;

    int ch;
    while ((ch = fgetc(f)) != EOF)
    {
        if (!isascii(ch))
        {
            fclose(f);
            return 0;
        }
    }
    fclose(f);
    return 1;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Hata: Eksik parametre girdiniz!\n");
        printf("Kullanım:\n  Birleştirme: tarsau -b dosya1 dosya2 -o cikis.sau\n  Açma: tarsau -a arsiv.sau hedef_dizin\n");
        return 1;
    }

    // =========================================================================
    // 1. MOD: DOSYALARI BİRLEŞTİRME (-b)
    // =========================================================================
    if (strcmp(argv[1], "-b") == 0)
    {
        printf("Birleştirme modu (-b) seçildi.\n");

        int dosya_sayisi = 0;
        int o_indeksi = -1;
        char cikis_dosya_adi[256] = "a.sau"; // Varsayılan çıktı adı

        for (int i = 2; i < argc; i++)
        {
            if (strcmp(argv[i], "-o") == 0)
            {
                o_indeksi = i;
                if (i + 1 < argc)
                {
                    strncpy(cikis_dosya_adi, argv[i + 1], 255); // ✅ GÜVENLI
                    cikis_dosya_adi[255] = '\0';
                }
                break;
            }
            dosya_sayisi++;
        }

        if (dosya_sayisi > 32)
        {
            printf("Hata: Giriş dosyası sayısı en fazla 32 olabilir!\n");
            return 1;
        }

        for (int i = 2; i < 2 + dosya_sayisi; i++)
        {
            if (!ascii_kontrol(argv[i]))
            {
                printf("%s giriş dosyasının formatı uyumsuzdur!\n", argv[i]);
                return 0;
            }
        }

        long long toplam_boyut = 0;
        char organizasyon_buffer[65536] = "";

        for (int i = 2; i < 2 + dosya_sayisi; i++)
        {
            struct stat st;
            if (stat(argv[i], &st) != 0)
            {
                printf("Hata: %s dosyasının bilgileri okunamadı!\n", argv[i]);
                return 1;
            }

            toplam_boyut += st.st_size;
            int izinler = st.st_mode & 0777;

            char gecici_kayit[512];
            snprintf(gecici_kayit, sizeof(gecici_kayit), "|%s,%o,%ld|", argv[i], izinler, st.st_size);
            strcat(organizasyon_buffer, gecici_kayit);
        }

        if (toplam_boyut > 200 * 1024 * 1024)
        {
            printf("Hata: Giriş dosyalarının toplam boyutu 200 MB'ı geçemez!\n");
            return 1;
        }

        FILE *cikis_f = fopen(cikis_dosya_adi, "w");
        if (cikis_f == NULL)
        {
            printf("Hata: Çıktı dosyası oluşturulamadı!\n");
            return 1;
        }

        long organizasyon_uzunlugu = strlen(organizasyon_buffer);
        fprintf(cikis_f, "%010ld", organizasyon_uzunlugu);
        fprintf(cikis_f, "%s", organizasyon_buffer);

        for (int i = 2; i < 2 + dosya_sayisi; i++)
        {
            FILE *girdi_f = fopen(argv[i], "r");
            if (girdi_f != NULL)
            {
                int ch;
                while ((ch = fgetc(girdi_f)) != EOF)
                {
                    fputc(ch, cikis_f);
                }
                fclose(girdi_f);
            }
        }

        fclose(cikis_f);
        printf("Dosyalar birleştirildi.\n");
    }

    // =========================================================================
    // 2. MOD: ARŞİVİ GERİ AÇMA (-a)
    // =========================================================================
    else if (strcmp(argv[1], "-a") == 0)
    {
        if (argc != 3 && argc != 4)
        {
            printf("Arşiv dosyası uygunsuz veya bozuk!\n");
            return 1;
        }

        char *arsiv_adi = argv[2];
        char *hedef_dizin = (argc == 4) ? argv[3] : NULL;

        // Arşiv dosyasını okuma modunda açıyoruz
        if (strlen(arsiv_adi) < 5 || strcmp(arsiv_adi + strlen(arsiv_adi) - 4, ".sau") != 0)
        {
            printf("Arşiv dosyası uygunsuz veya bozuk!\n");
            return 1;
        }

        FILE *arsiv_f = fopen(arsiv_adi, "r");
        if (arsiv_f == NULL)
        {
            printf("Arşiv dosyası uygunsuz veya bozuk!\n");
            return 1;
        }

        // 1) İlk 10 baytı oku ve organizasyon bölümünün uzunluğunu al
        char uzunluk_str[11] = {0};
        if (fread(uzunluk_str, 1, 10, arsiv_f) != 10)
        {
            printf("Arşiv dosyası uygunsuz veya bozuk!\n");
            fclose(arsiv_f);
            return 1;
        }
        long organizasyon_uzunlugu = atol(uzunluk_str);

        // 2) Organizasyon bölümünü hafızaya oku
        char *organizasyon = malloc(organizasyon_uzunlugu + 1);
        if (fread(organizasyon, 1, organizasyon_uzunlugu, arsiv_f) != organizasyon_uzunlugu)
        {
            printf("Arşiv dosyası uygunsuz veya bozuk!\n");
            free(organizasyon);
            fclose(arsiv_f);
            return 1;
        }
        organizasyon[organizasyon_uzunlugu] = '\0';

        // 3) Eğer hedef dizin belirtildiyse ve yoksa oluştur (mkdir)
        if (hedef_dizin != NULL)
        {
            // S_IRWXU | S_IRWXG | S_IRWXO -> Okuma, yazma, çalıştırma izinleri (0777)
            if (mkdir(hedef_dizin, 0777) != 0 && errno != EEXIST)
            {
                printf("Hata: Dizin oluşturulamadı: %s\n", hedef_dizin);
                free(organizasyon);
                fclose(arsiv_f);
                return 1;
            }
        }

        // 4) Organizasyon stringini ayrıştır ve dosyaları geri yaz
        // String içindeki verileri tek tek işlemek için geçici işaretçiler kullanıyoruz
        char *ptr = organizasyon;

        // Arşivdeki dosyaların içeriklerinin başladığı konumu kaydet
        long veri_baslangic_pos = ftell(arsiv_f);

        while ((ptr = strchr(ptr, '|')) != NULL)
        {
            ptr++; // '|' karakterini geç
            char *bitis = strchr(ptr, '|');
            if (bitis == NULL)
                break;

            // Tek bir kaydı ayır (Örn: "t1.txt,644,31")
            char kayit[512] = {0};
            long length = bitis - ptr;
            if (length > (long)sizeof(kayit) - 1)
                length = sizeof(kayit) - 1;
            strncpy(kayit, ptr, length);
            kayit[length] = '\0';
            ptr = bitis + 1; // Bir sonraki kayda hazırla

            // Kayıt içindeki alanları virgülle parçala
            char d_adi[256], d_izin_str[20];
            long d_boyut;

            sscanf(kayit, "%[^,],%[^,],%ld", d_adi, d_izin_str, &d_boyut);
            int d_izin = (int)strtol(d_izin_str, NULL, 8); // Octal stringi sayıya çevir

            // Hedef dosya yolunu belirle
            char tam_yol[512];
            if (hedef_dizin != NULL)
            {
                snprintf(tam_yol, sizeof(tam_yol), "%s/%s", hedef_dizin, d_adi);
            }
            else
            {
                strcpy(tam_yol, d_adi);
            }

            // Dosya verisini kopyalamak için arşiv dosyasındaki konumumuzu kaydet,
            // veri alanına zıpla, veriyi yaz ve eski konumumuza geri dön
            long anlik_organizasyon_pos = ftell(arsiv_f);
            fseek(arsiv_f, veri_baslangic_pos, SEEK_SET);

            FILE *yeni_dosya = fopen(tam_yol, "w");
            if (yeni_dosya != NULL)
            {
                char buffer[4096];
                long remaining = d_boyut;

                while (remaining > 0)
                {
                    size_t to_read = (remaining > 4096) ? 4096 : remaining;
                    size_t read = fread(buffer, 1, to_read, arsiv_f);
                    if (read == 0)
                        break;

                    fwrite(buffer, 1, read, yeni_dosya);
                    remaining -= read;
                }
                fclose(yeni_dosya);

                // Orijinal izinlerini geri tanımlıyoruz (chmod)
                chmod(tam_yol, d_izin);
            }

            // Bir sonraki dosyanın verisi bir öncekinin bittiği yerden devam edecek
            veri_baslangic_pos = ftell(arsiv_f);

            // Organizasyon bölümünü okumaya devam etmek için eski konumumuza dönüyoruz
            fseek(arsiv_f, anlik_organizasyon_pos, SEEK_SET);
        }

        free(organizasyon);
        fclose(arsiv_f);

        if (hedef_dizin != NULL)
        {
            printf("%s dizininde dosyalar açıldı.\n", hedef_dizin);
        }
        else
        {
            printf("Dosyalar geçerli dizinde açıldı.\n");
        }
    }

    else
    {
        printf("Hata: Geçersiz parametre '%s'! Lütfen -b veya -a kullanın.\n", argv[1]);
        return 1;
    }

    return 0;
}