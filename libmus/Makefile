CC=gcc

all: mus2midi

clean:
	rm -f *.o *~

mus2midi: mus2midi.o
	$(CC) mus2midi.o -o mus2midi

%.o: %.c
	$(CC) -g -c $< -o $@
