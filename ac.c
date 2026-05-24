#include "ac.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

int ac_modu(int argc, char *argv[])
{
    if (argc != 3 && argc != 4)
    {
        printf("Arşiv dosyası uygunsuz veya bozuk!\n");
        return 1;
    }

    char *arsiv_adi = argv[2];
    char *hedef_dizin = (argc == 4) ? argv[3] : NULL;

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

    char uzunluk_str[11] = {0};
    if (fread(uzunluk_str, 1, 10, arsiv_f) != 10)
    {
        printf("Arşiv dosyası uygunsuz veya bozuk!\n");
        fclose(arsiv_f);
        return 1;
    }
    long organizasyon_uzunlugu = atol(uzunluk_str);

    char *organizasyon = malloc(organizasyon_uzunlugu + 1);
    if (fread(organizasyon, 1, organizasyon_uzunlugu, arsiv_f) != (size_t)organizasyon_uzunlugu)
    {
        printf("Arşiv dosyası uygunsuz veya bozuk!\n");
        free(organizasyon);
        fclose(arsiv_f);
        return 1;
    }
    organizasyon[organizasyon_uzunlugu] = '\0';

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
    long veri_baslangic_pos = ftell(arsiv_f);

    while ((ptr = strchr(ptr, '|')) != NULL)
    {
        ptr++;
        char *bitis = strchr(ptr, '|');
        if (bitis == NULL)
            break;

        char kayit[512] = {0};
        long length = bitis - ptr;
        if (length > (long)sizeof(kayit) - 1)
            length = sizeof(kayit) - 1;
        strncpy(kayit, ptr, length);
        kayit[length] = '\0';
        ptr = bitis + 1;

        char d_adi[256], d_izin_str[20];
        long d_boyut;
        sscanf(kayit, "%[^,],%[^,],%ld", d_adi, d_izin_str, &d_boyut);
        int d_izin = (int)strtol(d_izin_str, NULL, 8);

        char tam_yol[512];
        if (hedef_dizin != NULL)
            snprintf(tam_yol, sizeof(tam_yol), "%s/%s", hedef_dizin, d_adi);
        else
            strcpy(tam_yol, d_adi);

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
            chmod(tam_yol, d_izin);
        }

        veri_baslangic_pos = ftell(arsiv_f);
        fseek(arsiv_f, anlik_organizasyon_pos, SEEK_SET);
    }

    free(organizasyon);
    fclose(arsiv_f);

    if (hedef_dizin != NULL)
        printf("%s dizininde dosyalar açıldı.\n", hedef_dizin);
    else
        printf("Dosyalar geçerli dizinde açıldı.\n");

    return 0;
}