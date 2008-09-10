OBJS=multiboot_c.o multiboot_asm.o console.o minilib.o
CC=gcc
CFLAGS=-nostdlib -I../include -I. -fno-builtin -nostdinc

all: multiboot

multiboot: $(OBJS)
	ld -o multiboot $(OBJS) -Ttext=0x100000
