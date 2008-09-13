CFLAGS=-I../include
CC=gcc
SMRAM_ICH2_OBJS=smram-linux-tool.o ../pci/pci-linux.o ../ich2/smram-ich2.o

smram-ich2: $(SMRAM_ICH2_OBJS)
	gcc -o smram-ich2 $(SMRAM_ICH2_OBJS)

clean:
	rm -f $(SMRAM_ICH2_OBJS) smram-ich2
