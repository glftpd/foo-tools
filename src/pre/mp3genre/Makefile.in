
include ../../Makefile.config

mp3genre:
	$(CC) -c $@.c -o $@.o

all: mp3genre

clean:
	rm -f *.o *~

release:
	mkdir -p $(RELEASEDIR)/pre/mp3genre
	cp *.c *.h $(RELEASEDIR)/pre/mp3genre

