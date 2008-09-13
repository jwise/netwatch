#include <io.h>

void __firstrun_start() {
/*
	puts("Current USB state is: ");
	puthex(pci_read16(0, 31, 2, 0xC0));
	puts(" ");
	puthex(pci_read16(0, 31, 4, 0xC0));
	puts("\n");
	puts("Current SMI state is: ");
	puthex(inl(0x830));
	puts("\n");
	puts("Current SMRAMC state is: ");
	puthex(pci_read8(0, 0, 0, 0x70));
	puts("\n");
*/

	/* Try really hard to shut up USB_LEGKEY. */
	pci_write16(0, 31, 2, 0xC0, pci_read16(0, 31, 2, 0xC0));
	pci_write16(0, 31, 2, 0xC0, 0);
	pci_write16(0, 31, 4, 0xC0, pci_read16(0, 31, 4, 0xC0));
	pci_write16(0, 31, 4, 0xC0, 0);

	/* Turn on the SMIs we want */
	outb(0x830, inb(0x830) | 0x41);
}

