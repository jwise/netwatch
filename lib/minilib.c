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

int memcmp (unsigned char *a2, unsigned char *a1, int bytes) {
	while (bytes--)
	{
		if (*(a2++) != *(a1++))
			return 1;
	}
	return 0;
}

int strcmp (unsigned char *a2, unsigned char *a1) {
	while (1) {
		if (*a2 != *a1) return 1;
		if (*a2 == 0) return 0;
		a1++;
		a2++;
	}
}

int strlen(char *c)
{
	int l = 0;
	while (*(c++))
		l++;
	return l;
}

void strcpy(unsigned char *a2, unsigned char *a1)
{
	do {
		*(a2++) = *a1;
	} while (*(a1++));
}

void puts(char *c)
{
	putbytes(c, strlen(c));
}

static char hexarr[] = "0123456789ABCDEF";
void tohex(unsigned char *s, unsigned long l)
{
	int i;
	for (i = 0; i < 8; i++)
	{
		s[i] = hexarr[l >> 28];
		l <<= 4;
	}
}

void puthex(unsigned long l)
{
	unsigned char d[9];
	d[8] = 0;
	tohex(d, l);
	puts(d);
}

