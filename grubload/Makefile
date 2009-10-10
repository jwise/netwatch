-include ../config.mk
-include ../hardware/ich2/config.mk

OBJS=main.o mbentry.o realmode.o loader.o ../pci/pci-raw.o ../lib/minilib.o ../lib/console.o $(CHIP_OBJS) output.o ../lib/sprintf.o ../lib/doprnt.o ../pci/pci.o
CC=$(CROSS)gcc
CFLAGS+=-nostdlib -I../include -I../include/raw -I. -D__RAW__ -fno-builtin -nostdinc -Wall -Werror -pedantic -std=gnu99

all: smmload

smmload: $(OBJS)
	$(CROSS)ld -o smmload $(OBJS) -Ttext=0x100000

realmode.bin: realmode.asm
	nasm -o realmode.bin realmode.asm

realmode.o: realmode.bin
	$(CROSS)objcopy -I binary -B i386 -O elf32-i386 realmode.bin realmode.o

clean:
	rm -f $(OBJS) realmode.bin smmload
