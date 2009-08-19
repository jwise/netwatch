/* minilib.h
 * Definitions for a very small libc
 * NetWatch system management mode administration console
 *
 * Copyright (c) 2008 Jacob Potter and Joshua Wise.  All rights reserved.
 * This program is free software; you can redistribute and/or modify it under
 * the terms found in the file LICENSE in the root of this source tree. 
 *
 */

#ifndef MINILIB_H
#define MINILIB_H

#include <stdarg.h>

extern void memcpy(void *dest, const void *src, int bytes);
extern void memset(void *dest, int data, int bytes);
extern void *memchr(const void *buf, char c, int maxlen);
extern void memmove(void *dest, void *src, int bytes);
extern int memcmp(const char *a2, const char *a1, int bytes);
extern int strcmp(const char *a2, const char *a1);
extern int strncmp(const char *a2, const char *a1, int n);
extern int strlen(const char *c);
extern void strcat(char *dest, char *src);
extern void strcpy(char *a2, const char *a1);
extern void puts(const char *c);
extern void tohex(char *s, unsigned long l);
extern void btohex(char *s, unsigned char c);
extern void puthex(unsigned long l);
extern int vsprintf(char *s, const char *fmt, va_list args);
extern int vsnprintf(char *s, int size, const char *fmt, va_list args);
extern int sprintf(char *s, const char *fmt, ...);
extern int snprintf(char *s, int size, const char *fmt, ...);
extern unsigned short htons(unsigned short in);
extern unsigned int htonl(unsigned int in);

#endif
