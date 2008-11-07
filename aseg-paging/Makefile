CC=gcc
CFLAGS=-I../include -I../include/raw \
       -I../lwip/src/include -I../lwip/src/include/ipv4 \
       -nostdlib -nostdinc -fno-builtin -D__RAW__ \
       -Wall -Werror -std=gnu99

STUBOBJS=entry.o pagingstub-asm.o pagingstub.o

OBJS=smi.o ../ich2/smi.o ../ich2/smram-ich2.o vga-overlay.o ../pci/pci-raw.o ../lib/minilib.o ../lib/doprnt.o ../lib/sprintf.o ../lib/console.o ../lib/serial.o

all: aseg.elf

aseg.elf: aseg.lds $(STUBOBJS) $(OBJS)
	ld -o aseg.elf -T aseg.lds $(STUBOBJS) $(OBJS)

clean:
	rm -f $(OBJS) aseg.elf entry.bin entry.o

%.o: %.asm
	nasm -o $@.bin $<
	objcopy -I binary -B i386 -O elf32-i386 $@.bin $@
	rm -f $@.bin
