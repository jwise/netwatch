CFLAGS=-I../include
CC=gcc
SMRAM_ICH2_OBJS=smram-linux-tool.o ../pci/pci-linux.o ../ich2/smram-ich2.noraw.o

all: smram-ich2 port

%.noraw.o: %.c
	gcc $(CFLAGS) -c -o $@ $<

smram-ich2: $(SMRAM_ICH2_OBJS)
	gcc $(CFLAGS) -o smram-ich2 $(SMRAM_ICH2_OBJS)

port: port.o
	gcc -o port port.o

clean:
	rm -f $(SMRAM_ICH2_OBJS) smram-ich2
