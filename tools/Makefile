-include ../config.mk

CFLAGS=-I../include
CC=$(CROSS)gcc
SMRAM_ICH2_OBJS=smram-linux-tool.o ../pci/pci-linux.o ../ich2/smram-ich2.noraw.o
PCI_OBJS=pci.o ../pci/pci-linux.o
POKE_RLS_OBJS=poke-rls.o poke-rls-asm.o ../pci/pci-linux.o
FROB_RLS_OBJS=frob-rls.o poke-rls-asm.o ../pci/pci-linux.o

all: smram-ich2 port pci poke-rls

%.noraw.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

smram-ich2: $(SMRAM_ICH2_OBJS)
	$(CC) $(CFLAGS) -o smram-ich2 $(SMRAM_ICH2_OBJS)

port: port.o
	$(CC) -o port port.o

elfload: elfload.o
	$(CC) -o elfload elfload.o

pci: $(PCI_OBJS)
	$(CC) $(CFLAGS) -o pci $(PCI_OBJS)

frob-rls: $(FROB_RLS_OBJS)
	$(CC) $(CFLAGS) -o frob-rls $(FROB_RLS_OBJS)

clean:
	rm -f $(SMRAM_ICH2_OBJS) smram-ich2

poke:
