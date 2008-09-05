char counter = 0;

#define outb(port, val) \
({ asm volatile("outb %0, %%dx" : : "a" ((unsigned char)(val)) , "d" ((unsigned short)(port))); })

void __start (void)
{
	counter++;
	outb (0x80, counter);
}

