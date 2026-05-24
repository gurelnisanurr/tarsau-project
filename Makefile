CC = gcc
CFLAGS = -Wall -Wextra

OBJS = main.o birles.o ac.o utils.o

all: tarsau

tarsau: $(OBJS)
	$(CC) $(CFLAGS) -o tarsau $(OBJS)

main.o: main.c birles.h ac.h
	$(CC) $(CFLAGS) -c main.c

birles.o: birles.c birles.h utils.h
	$(CC) $(CFLAGS) -c birles.c

ac.o: ac.c ac.h
	$(CC) $(CFLAGS) -c ac.c

utils.o: utils.c utils.h
	$(CC) $(CFLAGS) -c utils.c

clean:
	rm -f tarsau $(OBJS) test_arsiv.sau a.sau
	rm -rf cikti_klasoru