CFLAGS=-I../include

all: aseg.elf

aseg.bin: aseg.asm
	nasm -o aseg.bin aseg.asm

aseg.o: aseg.bin
	objcopy -I binary -B i386 -O elf32-i386 aseg.bin aseg.o

aseg.elf: aseg.o aseg.lds counter.o ../pci/pci-smm.o
	ld -o aseg.elf -T aseg.lds counter.o ../pci/pci-smm.o
