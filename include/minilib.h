#ifndef MINILIB_H
#define MINILIB_H

#include <stdarg.h>

extern void memcpy(void *dest, const void *src, int bytes);
extern void memmove(void *dest, void *src, int bytes);
extern int memcmp(const char *a2, const char *a1, int bytes);
extern int strcmp(const char *a2, const char *a1);
extern int strlen(const char *c);
extern void strcpy(char *a2, const char *a1);
extern void puts(const char *c);
extern void tohex(char *s, unsigned long l);
extern void puthex(unsigned long l);
extern int vsprintf(char *s, const char *fmt, va_list args);
extern int vsnprintf(char *s, int size, const char *fmt, va_list args);
extern int sprintf(char *s, const char *fmt, ...);
extern int snprintf(char *s, int size, const char *fmt, ...);
extern unsigned short htons(unsigned short in);

#endif
