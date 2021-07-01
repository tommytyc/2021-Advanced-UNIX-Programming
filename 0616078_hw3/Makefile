CC = gcc
ASSEM = yasm
AFLAGS = -f elf64 -DYASM -D__x86_64__ -DPIC
CFLAGS = -c -g -Wall -fno-stack-protector

.PHONY: clean test

all: libmini.so start.o

start.o: start.asm
	$(ASSEM) $(AFLAGS) start.asm -o start.o

libmini.so: libmini64.asm libmini.c
	$(ASSEM) $(AFLAGS) libmini64.asm -o libmini64.o
	$(CC) $(CFLAGS) -fPIC -nostdlib libmini.c
	ld -shared -o libmini.so libmini64.o libmini.o

test: start.o test.c
	$(CC) $(CFLAGS) -nostdlib -I. -I.. -DUSEMINI test.c
	ld -m elf_x86_64 --dynamic-linker /lib64/ld-linux-x86-64.so.2 -o test test.o start.o -L. -L.. -lmini

clean:
	rm -f a.out test *.o libmini.so