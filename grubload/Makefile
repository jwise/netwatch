OBJS=multiboot_c.o multiboot_asm.o realmode.o loader.o ../pci/pci-raw.o ../lib/minilib.o ../lib/console.o ../ich2/smram-ich2.o ../ich2/smi.o
CC=gcc
CFLAGS=-nostdlib -I../include -I../include/raw -I. -D__RAW__ -fno-builtin -nostdinc

all: multiboot

multiboot: $(OBJS)
	ld -o multiboot $(OBJS) -Ttext=0x100000

realmode.bin: realmode.asm
	nasm -o realmode.bin realmode.asm

realmode.o: realmode.bin
	objcopy -I binary -B i386 -O elf32-i386 realmode.bin realmode.o

clean:
	rm -f $(OBJS) realmode.bin multiboot
