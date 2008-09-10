OBJS=multiboot_c.o multiboot_asm.o console.o minilib.o realmode.o loader.o
CC=gcc
CFLAGS=-nostdlib -I../include -I. -fno-builtin -nostdinc

all: multiboot

multiboot: $(OBJS)
	ld -o multiboot $(OBJS) -Ttext=0x100000

realmode.bin: realmode.asm
	nasm -o realmode.bin realmode.asm

realmode.o: realmode.bin
	objcopy -I binary -B i386 -O elf32-i386 realmode.bin realmode.o
