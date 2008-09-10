#include "console.h"

void memcpy(unsigned char *a2, unsigned char *a1, int bytes)
{
	while (bytes--)
		*(a2++) = *(a1++);
}

void memmove(unsigned char *dest, unsigned char *src, int bytes)
{
	if ((dest > src) && (dest <= (src + bytes)))
	{
		/* do it backwards! */
		dest += bytes;
		src += bytes;
		while (bytes--)
			*(--dest) = *(--src);
	} else
		while (bytes--)
			*(dest++) = *(src++);
}

int strlen(char *c)
{
	int l = 0;
	while (*(c++))
		l++;
	return l;
}

void puts(char *c)
{
	putbytes(c, strlen(c));
}

static char hexarr[] = "0123456789ABCDEF";
void puthex(unsigned long l)
{
	int i;
	for (i = 0; i < 8; i++)
	{
		putbyte(hexarr[l >> 28]);
		l <<= 4;
	}
}
