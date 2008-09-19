#ifndef MINILIB_H
#define MINILIB_H

void memcpy(void *dest, void *src, int bytes);
void memmove(void *dest, void *src, int bytes);
int strcmp(const char *a2, const char *a1);
void strcpy(char *a2, const char *a1);
void tohex(char *s, unsigned long l);

void puts(const char *c);
void puthex(unsigned long l);

#endif
