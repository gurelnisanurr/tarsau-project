#include <stdio.h>
#include <string.h>
#include "birles.h"
#include "ac.h"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Hata: Eksik parametre girdiniz!\n");
        printf("Kullanım:\n  Birleştirme: tarsau -b dosya1 dosya2 -o cikis.sau\n  Açma: tarsau -a arsiv.sau hedef_dizin\n");
        return 1;
    }

    if (strcmp(argv[1], "-b") == 0)
        return birles_modu(argc, argv);
    else if (strcmp(argv[1], "-a") == 0)
        return ac_modu(argc, argv);
    else
    {
        printf("Hata: Geçersiz parametre '%s'! Lütfen -b veya -a kullanın.\n", argv[1]);
        return 1;
    }
}