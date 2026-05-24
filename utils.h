/* ============================================================
 * utils.h — yardımcı fonksiyonların başlık dosyası
 * ============================================================ */

#ifndef UTILS_H
#define UTILS_H

/* Verilen dosyadaki tüm karakterlerin ASCII olup olmadığını kontrol eder.
 * Dönüş: 1 → tümü ASCII, 0 → ASCII dışı karakter var ya da dosya açılamadı */
int ascii_kontrol(const char *dosya_adi);

#endif