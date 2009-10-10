CFLAGS += -DNETWATCH_ICH2 -I../hardware/ich2
CHIP_OBJS += ../hardware/ich2/ich2-timer.o \
             ../hardware/ich2/smi.o \
             ../hardware/ich2/smram-ich2.o
