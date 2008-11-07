CC=gcc
CFLAGS=-I../include -I../include/raw -I../lwip/src/include -I../lwip/src/include/ipv4 -nostdlib -nostdinc -fno-builtin -D__RAW__ -Wall -Werror -pedantic -ansi -std=gnu99

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

OBJS=counter.o firstrun.o ../pci/pci-raw.o ../lib/minilib.o ../lib/console.o \
	../ich2/smram-ich2.o ../ich2/smi.o vga-overlay.o packet.o \
	../lib/sprintf.o ../lib/doprnt.o ../pci/pci.o ../net/net.o \
	../ich2/ich2-timer.o ../pci/pci-bother.o ../net/3c90x.o keyboard.o \
	../lib/serial.o $(LWIP_OBJS)

all: aseg.elf

aseg.elf: aseg.lds aseg.o $(OBJS)
	ld -o aseg.elf -T aseg.lds $(OBJS)

clean:
	rm -f $(OBJS) aseg.elf aseg.bin aseg.o


%.o: %.asm
	nasm -o $@.bin $<
	objcopy -I binary -B i386 -O elf32-i386 $@.bin $@
	rm -f $@.bin
