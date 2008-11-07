#include <minilib.h>
#include <io.h>

#define SER_BASE 0x3F8
#define SER_THR 0x0
#define SER_RBR 0x0
#define SER_DLL 0x0
#define SER_IER 0x1
#define SER_DLM 0x1
#define SER_IIR 0x2
#define SER_FCR 0x2
#define SER_LCR 0x3
#define SER_LCR_DLAB 0x80
#define SER_MCR 0x4
#define SER_LSR 0x5
#define SER_LSR_THR_EMPTY 0x20
#define SER_MSR 0x6
#define SER_SR 0x7

#define SER_BAUD_BASE 115200
#define SER_BAUD_REQ 115200

void _outb(unsigned short port, unsigned char d)
{
	outb(SER_BASE + port, d);
}

unsigned char _inb(unsigned short port)
{
	return inb(SER_BASE + port);
}

void serial_init()
{
	unsigned short baud = SER_BAUD_REQ / SER_BAUD_BASE;
	_outb(SER_LCR, inb(SER_LCR) | SER_LCR_DLAB);
	_outb(SER_DLL, baud & 0xFF);
	_outb(SER_DLM, baud >> 8);
	_outb(SER_LCR, inb(SER_LCR) & ~SER_LCR_DLAB);
	_outb(SER_IER, 0x0);
	_outb(SER_FCR, 0x0);	/* FIFOs off */
	_outb(SER_LCR, 0x03);	/* 8 data bits, one stop bit, no parity */
}

void serial_tx(unsigned char c)
{
	while (!(_inb(SER_LSR) & SER_LSR_THR_EMPTY))
		;
	_outb(SER_THR, c);
}
