#ifndef __IO_H
#define __IO_H

#define inb(port) ({ unsigned char val; asm volatile("inb %%dx, %0" : "=a" ((unsigned char)(val)) : "d" ((unsigned short)(port))); val; })
#define inw(port) ({ unsigned short val; asm volatile("inw %%dx, %0" : "=a" ((unsigned short)(val)) : "d" ((unsigned short)(port))); val; })
#define inl(port) ({ unsigned long val; asm volatile("inl %%dx, %0" : "=a" ((unsigned long)(val)) : "d" ((unsigned short)(port))); val; })
#define outb(port, val) ({ asm volatile("outb %0, %%dx" : : "a" ((unsigned char)(val)) , "d" ((unsigned short)(port))); })
#define outw(port, val) ({ asm volatile("outw %0, %%dx" : : "a" ((unsigned short)(val)) , "d" ((unsigned short)(port))); })
#define outl(port, val) ({ asm volatile("outl %0, %%dx" : : "a" ((unsigned long)(val)) , "d" ((unsigned short)(port))); })

#endif
