CC=gcc
CFLAGS=-I../include -I../include/raw -nostdlib -nostdinc -fno-builtin -D__RAW__
OBJS=counter.o ../pci/pci-raw.o ../lib/minilib.o ../lib/console.o

all: aseg.elf

aseg.bin: aseg.asm
	nasm -o aseg.bin aseg.asm

aseg.o: aseg.bin
	objcopy -I binary -B i386 -O elf32-i386 aseg.bin aseg.o

aseg.elf: aseg.lds aseg.o $(OBJS)
	ld -o aseg.elf -T aseg.lds $(OBJS)

clean:
	rm -f $(OBJS) aseg.elf aseg.bin aseg.o
