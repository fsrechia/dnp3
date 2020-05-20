CC=gcc
CFLAGS=-I. -Wall
DEPS = dnp3.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

outstation: dnp3.o outstation.o
	$(CC) -o outstation dnp3.o outstation.o

master: dnp3.o master.o
	$(CC) -o master dnp3.o master.o

.PHONY: clean

clean:
	rm -f *.o *~ core master outstation