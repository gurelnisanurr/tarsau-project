#include "utils.h"
#include <stdio.h>
#include <ctype.h>

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