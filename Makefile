# ============================================================
# Makefile — tarsau projesini derleme kuralları
# Derleyici: gcc   |   Bayraklar: tüm uyarıları etkinleştir
# ============================================================

CC = gcc
CFLAGS = -Wall -Wextra

# Bağlanacak nesne dosyalarının listesi
OBJS = main.o birles.o ac.o utils.o

# Varsayılan hedef: tarsau çalıştırılabilirini oluştur
all: tarsau

# Tüm nesne dosyalarını birleştirerek nihai çalıştırılabiliri üret
tarsau: $(OBJS)
	$(CC) $(CFLAGS) -o tarsau $(OBJS)

# Her kaynak dosyası için ayrı derleme kuralları
main.o: main.c birles.h ac.h
	$(CC) $(CFLAGS) -c main.c

birles.o: birles.c birles.h utils.h
	$(CC) $(CFLAGS) -c birles.c

ac.o: ac.c ac.h
	$(CC) $(CFLAGS) -c ac.c

utils.o: utils.c utils.h
	$(CC) $(CFLAGS) -c utils.c

# Derleme çıktılarını temizle
clean:
	rm -f tarsau $(OBJS) test_arsiv.sau a.sau
	rm -rf cikti_klasoru