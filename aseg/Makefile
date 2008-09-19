CC=gcc
CFLAGS=-I../include -I../include/raw -nostdlib -nostdinc -fno-builtin -D__RAW__ -Wall -Werror -pedantic -ansi -std=gnu99
OBJS=counter.o firstrun.o ../pci/pci-raw.o ../lib/minilib.o ../lib/console.o ../ich2/smram-ich2.o ../ich2/smi.o vga-overlay.o ../lib/sprintf.o ../lib/doprnt.o

all: aseg.elf

aseg.elf: aseg.lds aseg.o $(OBJS)
	ld -o aseg.elf -T aseg.lds $(OBJS)

clean:
	rm -f $(OBJS) aseg.elf aseg.bin aseg.o


%.o: %.asm
	nasm -o $@.bin $<
	objcopy -I binary -B i386 -O elf32-i386 $@.bin $@
	rm -f $@.bin
