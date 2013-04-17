CC=gcc

CFLAGS=-g -Wall
LFLAGS=

SOURCES=../hash/hash.c ../hash/stash.c ../cssdom.c main.c
EXECUTABLE=css

OBJECTS=$(SOURCES:.c=.o)

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -rf *.o *.a $(EXECUTABLE) 