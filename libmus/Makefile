CC=gcc
CFLAGS=-g

OBJS= \
	mus2midi.o \
	midi.o \
	mus.o

all: mus2midi

clean:
	rm -f *.o *~
	rm mus2midi

mus2midi: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o mus2midi

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
