#include "birles.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

int birles_modu(int argc, char *argv[])
{
    printf("Birleştirme modu (-b) seçildi.\n");

    int dosya_sayisi = 0;
    char cikis_dosya_adi[256] = "a.sau";

    for (int i = 2; i < argc; i++)
    {
        if (strcmp(argv[i], "-o") == 0)
        {
            if (i + 1 < argc)
            {
                strncpy(cikis_dosya_adi, argv[i + 1], 255);
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
                fputc(ch, cikis_f);
            fclose(girdi_f);
        }
    }

    fclose(cikis_f);
    printf("Dosyalar birleştirildi.\n");
    return 0;
}