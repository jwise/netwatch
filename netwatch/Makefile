-include ../config.mk
-include ../hardware/ich2/config.mk

CC=$(CROSS)gcc
CFLAGS+=-I../include -I../net -I../include/raw \
        -I../lwip/src/include -I../lwip/src/include/ipv4 \
        -nostdlib -nostdinc -fno-builtin -D__RAW__ \
        -Wall -Werror -std=gnu99 -Wstrict-aliasing=2 \
        -O1 -fno-merge-constants -fno-strict-aliasing

STUBOBJS=entry.o pagingstub-asm.o pagingstub.o

LWIP_OBJS = \
	../lwip/src/core/dhcp.o \
	../lwip/src/core/dns.o \
	../lwip/src/core/init.o \
	../lwip/src/core/ipv4/autoip.o \
	../lwip/src/core/ipv4/icmp.o \
	../lwip/src/core/ipv4/igmp.o \
	../lwip/src/core/ipv4/inet.o \
	../lwip/src/core/ipv4/inet_chksum.o \
	../lwip/src/core/ipv4/ip.o \
	../lwip/src/core/ipv4/ip_addr.o \
	../lwip/src/core/ipv4/ip_frag.o \
	../lwip/src/core/mem.o \
	../lwip/src/core/memp.o \
	../lwip/src/core/netif.o \
	../lwip/src/core/pbuf.o \
	../lwip/src/core/raw.o \
	../lwip/src/core/stats.o \
	../lwip/src/core/sys.o \
	../lwip/src/core/tcp.o \
	../lwip/src/core/tcp_in.o \
	../lwip/src/core/tcp_out.o \
	../lwip/src/core/udp.o \
	../lwip/src/netif/etharp.o \
	../lwip/src/netif/ethernetif.o


OBJS =	$(CHIP_OBJS) \
	../pci/pci.o \
	../pci/pci-raw.o \
	../pci/pci-bother.o \
	../net/net.o \
	../net/http/fs.o \
	../net/http/httpd.o \
	../hardware/net/3c90x.o \
	../net/rfb.o \
	../hardware/video/tnt2.o \
	../hardware/video/fb.o \
	../hardware/video/generic.o \
	../hardware/video/text.o \
	drivers.o \
	../lib/minilib.o \
	../lib/doprnt.o \
	../lib/sprintf.o \
	../lib/console.o \
	../lib/serial.o \
	../lib/crc32.o \
	../lib/demap.o \
	../lib/state.o \
	../lib/cpuid.o \
	keyboard.o \
	packet.o \
	$(LWIP_OBJS) \
	smi.o \
	vga-overlay.o \
	main.o \
	traps.o \
	firstrun.o


all: aseg.elf

aseg.elf: netwatch-large.lds $(STUBOBJS) $(OBJS)
	$(CROSS)ld -o aseg.elf -T netwatch-large.lds $(STUBOBJS) $(OBJS)

clean:
	rm -f $(OBJS) aseg.elf entry.bin entry.o

%.o: %.asm
	nasm -o $@.bin $<
	$(CROSS)objcopy -I binary -B i386 -O elf32-i386 $@.bin $@
	rm -f $@.bin
