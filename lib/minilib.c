#include "console.h"
#include <minilib.h>

void memcpy(void *dest, const void *src, int bytes)
{
	char *cdest = dest;
	const char *csrc = src;
	while (bytes--)
		*(cdest++) = *(csrc++);
}

void memset(void *dest, int data, int bytes)
{
	unsigned char *cdest = dest;
	while (bytes--)
		*(cdest++) = (unsigned char)data;
}

void memmove(void *dest, void *src, int bytes)
{
	char * cdest = dest;
	char * csrc = src;
	if ((cdest > csrc) && (cdest <= (csrc + bytes)))
	{
		/* do it backwards! */
		cdest += bytes;
		csrc += bytes;
		while (bytes--)
			*(--cdest) = *(--csrc);
	} else
		while (bytes--)
			*(cdest++) = *(csrc++);
}

int memcmp (const char *a2, const char *a1, int bytes) {
	while (bytes--)
	{
		if (*(a2++) != *(a1++))
			return 1;
	}
	return 0;
}

int strcmp (const char *a2, const char *a1) {
	while (1) {
		if (*a2 != *a1) return 1;
		if (*a2 == 0) return 0;
		a1++;
		a2++;
	}
}

int strncmp (const char *a2, const char *a1, int n) {
	while (n--) {
		if (*a2 != *a1) return 1;
		if (*a2 == 0) return 0;
		a1++;
		a2++;
	}
	return 0;
}

int strlen(const char *c)
{
	int l = 0;
	while (*(c++))
		l++;
	return l;
}

void strcpy(char *a2, const char *a1)
{
	do {
		*(a2++) = *a1;
	} while (*(a1++));
}

void puts(const char *c)
{
	putbytes(c, strlen(c));
}

static char hexarr[] = "0123456789ABCDEF";
void tohex(char *s, unsigned long l)
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
	char d[9];
	d[8] = 0;
	tohex(d, l);
	puts(d);
}

unsigned short htons(unsigned short in)
{
	return (in >> 8) | (in << 8);
}

unsigned int htonl(unsigned int in)
{
	return ((in & 0xff) << 24) |
	       ((in & 0xff00) << 8) |
	       ((in & 0xff0000UL) >> 8) |
	       ((in & 0xff000000UL) >> 24);
}
