#include <sys/io.h>
#include <string.h>

int main(int argc, char **argv)
{
	unsigned int port;
	unsigned char type = 'b';
	unsigned int datum;
	
	if (iopl(3) < 0)
	{
		perror("iopl");
		return 1;
	}
	
	if ((argc < 2) || (argc > 4))
	{
	usage:
		printf("usage: %s port [b|w|l [datum]]\n", argv[0]);
		return 2;
	}
	
	port = strtoul(argv[1], NULL, 0);
	
	if (argc > 2)
		type = *argv[2];
	
	if (argc > 3)
	{
		datum = strtoul(argv[3], NULL, 0);
		switch (type)
		{
		case 'b':
			datum &= 0xFF;
			outb(datum, port);
			printf("Wrote byte 0x%02x to port 0x%04x\n", datum, port);
			break;
		case 'w':
			datum &= 0xFFFF;
			outw(datum, port);
			printf("Wrote word 0x%04x to port 0x%04x\n", datum, port);
			break;
		case 'l':
			outb(datum, port);
			printf("Wrote long 0x%08x to port 0x%04x\n", datum, port);
			break;
		default:
			goto usage;
		}
	} else {
		switch(type)
		{
		case 'b':
			datum = inb(port);
			printf("Read byte 0x%02x from port 0x%04x\n", datum, port);
			break;
		case 'w':
			datum = inw(port);
			printf("Read word 0x%04x from port 0x%04x\n", datum, port);
			break;
		case 'l':
			datum = inl(port);
			printf("Read long 0x%08x from port 0x%04x\n", datum, port);
			break;
		default:
			goto usage;
		}
	}
}
