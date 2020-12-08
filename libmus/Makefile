CC=gcc
CFLAGS=-g

C_SOURCES = $(wildcard *.c)
HEADERS = ${wildcard *.h}
OBJS = ${C_SOURCES:.c=.o}

all: mus2midi

clean:
	rm -f *.o
	rm mus2midi

mus2midi: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o mus2midi

%.o: %.c ${HEADERS}
	$(CC) $(CFLAGS) -c $< -o $@
