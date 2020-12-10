CC=gcc
CFLAGS=-g

C_SOURCES = $(wildcard *.c)
HEADERS = ${wildcard *.h}
OBJS = ${C_SOURCES:.c=.o}
LIBS = -lSDL2 -lSDL2_mixer

all: mus2midi

clean:
	rm -f *.o
	rm mus2midi test.mid

mus2midi: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o mus2midi $(LIBS)

%.o: %.c ${HEADERS}
	$(CC) $(CFLAGS) -c $< -o $@
