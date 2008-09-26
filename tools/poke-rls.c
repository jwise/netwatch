#include <sys/io.h>
#include <reg-82801b.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

static uint16_t _get_PMBASE()
{
	static long pmbase = -1;

	if (pmbase == -1) 
		pmbase = pci_read32(ICH2_LPC_BUS, ICH2_LPC_DEV, ICH2_LPC_FN, ICH2_LPC_PCI_PMBASE) & ICH2_PMBASE_MASK;

	return pmbase;
}


extern unsigned int poke(unsigned long addr);

int main(int argc, char **argv)
{
	unsigned int res;

	if (iopl(3) < 0)
	{
		perror("iopl");
		return 1;
	}
	
	res = poke(_get_PMBASE() + 0x04);
	printf("found %p\n", res);
}
