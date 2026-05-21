all:
	gcc main.c -o tarsau

clean:
	rm -f tarsau test_arsiv.sau a.sau
	rm -rf cikti_klasoru