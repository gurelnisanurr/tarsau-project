/* ============================================================
 * ac.c — .sau arşiv dosyasını açma (extraction) modülü
 *
 * .sau dosya formatı:
 *   [10 bayt: organizasyon bölümü uzunluğu (ASCII sayı)]
 *   [organizasyon bölümü: |dosyaadi,izin_oktal,boyut| kayıtları]
 *   [ham dosya verisi — kayıtların sırasıyla ardışık baytları]
 * ============================================================ */

#include "ac.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

int ac_modu(int argc, char *argv[])
{
    /* Parametre sayısını doğrula: tarsau -a arsiv.sau [hedef_dizin] */
    if (argc != 3 && argc != 4)
    {
        printf("Arşiv dosyası uygunsuz veya bozuk!\n");
        return 1;
    }

    char *arsiv_adi = argv[2];
    /* Hedef dizin belirtilmemişse NULL bırak (geçerli dizine açılacak) */
    char *hedef_dizin = (argc == 4) ? argv[3] : NULL;

    /* Arşiv dosyasının .sau uzantısına sahip olup olmadığını kontrol et */
    if (strlen(arsiv_adi) < 5 || strcmp(arsiv_adi + strlen(arsiv_adi) - 4, ".sau") != 0)
    {
        printf("Arşiv dosyası uygunsuz veya bozuk!\n");
        return 1;
    }

    /* Arşiv dosyasını oku modunda aç */
    FILE *arsiv_f = fopen(arsiv_adi, "r");
    if (arsiv_f == NULL)
    {
        printf("Arşiv dosyası uygunsuz veya bozuk!\n");
        return 1;
    }

    /* İlk 10 baytı oku: organizasyon bölümünün uzunluğunu verir */
    char uzunluk_str[11] = {0};
    if (fread(uzunluk_str, 1, 10, arsiv_f) != 10)
    {
        printf("Arşiv dosyası uygunsuz veya bozuk!\n");
        fclose(arsiv_f);
        return 1;
    }
    long organizasyon_uzunlugu = atol(uzunluk_str);

    /* Organizasyon bölümünü (dosya meta verilerini) belleğe yükle */
    char *organizasyon = malloc(organizasyon_uzunlugu + 1);
    if (fread(organizasyon, 1, organizasyon_uzunlugu, arsiv_f) != (size_t)organizasyon_uzunlugu)
    {
        printf("Arşiv dosyası uygunsuz veya bozuk!\n");
        free(organizasyon);
        fclose(arsiv_f);
        return 1;
    }
    organizasyon[organizasyon_uzunlugu] = '\0';

    /* Hedef dizin belirtildiyse oluştur (zaten varsa devam et) */
    if (hedef_dizin != NULL)
    {
        if (mkdir(hedef_dizin, 0777) != 0 && errno != EEXIST)
        {
            printf("Hata: Dizin oluşturulamadı: %s\n", hedef_dizin);
            free(organizasyon);
            fclose(arsiv_f);
            return 1;
        }
    }

    char *ptr = organizasyon;
    /* Ham veri bölümünün dosya içindeki başlangıç konumunu kaydet */
    long veri_baslangic_pos = ftell(arsiv_f);

    /* Organizasyon bölümündeki her |...| kaydını sırayla işle */
    while ((ptr = strchr(ptr, '|')) != NULL)
    {
        ptr++; /* '|' karakterini atla */
        char *bitis = strchr(ptr, '|'); /* Kaydın bitiş sınırını bul */
        if (bitis == NULL)
            break;

        /* Kaydı geçici tampona kopyala */
        char kayit[512] = {0};
        long length = bitis - ptr;
        if (length > (long)sizeof(kayit) - 1)
            length = sizeof(kayit) - 1;
        strncpy(kayit, ptr, length);
        kayit[length] = '\0';
        ptr = bitis + 1; /* Bir sonraki kayda geç */

        /* Kaydı ayrıştır: dosya_adi, izin (oktal), boyut */
        char d_adi[256], d_izin_str[20];
        long d_boyut;
        sscanf(kayit, "%[^,],%[^,],%ld", d_adi, d_izin_str, &d_boyut);
        int d_izin = (int)strtol(d_izin_str, NULL, 8); /* Oktal izni tam sayıya çevir */

        /* Hedef dosyanın tam yolunu oluştur */
        char tam_yol[512];
        if (hedef_dizin != NULL)
            snprintf(tam_yol, sizeof(tam_yol), "%s/%s", hedef_dizin, d_adi);
        else
            strcpy(tam_yol, d_adi);

        /* Organizasyon imlecinin konumunu sakla; veri okumak için başa dön */
        long anlik_organizasyon_pos = ftell(arsiv_f);
        fseek(arsiv_f, veri_baslangic_pos, SEEK_SET);

        /* Dosyayı oluştur ve arşivden içeriğini yaz */
        FILE *yeni_dosya = fopen(tam_yol, "w");
        if (yeni_dosya != NULL)
        {
            char buffer[4096];
            long remaining = d_boyut;
            while (remaining > 0)
            {
                /* Kalan baytları 4 KB'lık parçalar halinde oku ve yaz */
                size_t to_read = (remaining > 4096) ? 4096 : remaining;
                size_t read = fread(buffer, 1, to_read, arsiv_f);
                if (read == 0)
                    break;
                fwrite(buffer, 1, read, yeni_dosya);
                remaining -= read;
            }
            fclose(yeni_dosya);
            chmod(tam_yol, d_izin); /* Özgün dosya izinlerini geri yükle */
        }

        /* Bir sonraki dosyanın veri başlangıç konumunu güncelle */
        veri_baslangic_pos = ftell(arsiv_f);
        /* Organizasyon bölümündeki ilerlemeyi geri yükle */
        fseek(arsiv_f, anlik_organizasyon_pos, SEEK_SET);
    }

    free(organizasyon);
    fclose(arsiv_f);

    /* Açma işleminin nereye yapıldığını kullanıcıya bildir */
    if (hedef_dizin != NULL)
        printf("%s dizininde dosyalar açıldı.\n", hedef_dizin);
    else
        printf("Dosyalar geçerli dizinde açıldı.\n");

    return 0;
}