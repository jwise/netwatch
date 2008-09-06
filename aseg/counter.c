#include <io.h>

char counter = 0;
unsigned long pcisave;
unsigned char vgasave;
unsigned char thestr[512];

void memcpy(char *dst, char *src, int c)
{
	while (c--)
		*(dst++) = *(src++);
}

void strcpy(char *dst, char *src)
{
	while (*src)
		*(dst++) = *(src++);
}

unsigned char vgaread(unsigned char idx)
{
	outb(0x3D4, idx);
	inb(0x3D5);
}

void strblit(char *src)
{
	char *destp = (char*)(0xB8000 | (vgaread(0xC) << 5) | (vgaread(0xD) << 1));
	while (*src)
	{
		*(destp++) = *(src++);
		*(destp++) = 0x1F;
	}
}

void __start (void)
{
	unsigned char smramc;
	
	pcisave = inl(0xCF8);
	vgasave = inb(0x3D4);

	counter++;
	outb(0x80, counter);
	
	strcpy(thestr, "15-412!");
	
	smramc = pci_read8(0, 0, 0, 0x70);
	pci_write8(0, 0, 0, 0x70, (smramc & 0xF3) | 0x08);
	strblit(thestr);
	pci_write8(0, 0, 0, 0x70, smramc);
	
	outl(0xCF8, pcisave);
	outb(0x3D4, vgasave);
}

