CFLAGS += -DNETWATCH_ICH2 -I../ich2
CHIP_OBJS += ../ich2/ich2-timer.o \
             ../ich2/smi.o \
             ../ich2/smram-ich2.o
