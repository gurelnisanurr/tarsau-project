/* ============================================================
 * utils.c — yardımcı işlevler
 * ============================================================ */

#include "utils.h"
#include <stdio.h>
#include <ctype.h>

/* Dosyadaki tüm karakterlerin geçerli ASCII (0-127) olup olmadığını kontrol eder.
 * Dosya açılamazsa veya ASCII dışı karakter bulunursa 0 (başarısız) döndürür;
 * tüm karakterler ASCII ise 1 (başarılı) döndürür. */
int ascii_kontrol(const char *dosya_adi)
{
    FILE *f = fopen(dosya_adi, "r");
    if (f == NULL)
        return 0; /* Dosya açılamadı */

    int ch;
    while ((ch = fgetc(f)) != EOF)
    {
        /* ASCII olmayan bir bayt bulunursa dosyayı kapat ve başarısız dön */
        if (!isascii(ch))
        {
            fclose(f);
            return 0;
        }
    }
    fclose(f);
    return 1; /* Tüm baytlar geçerli ASCII */
}