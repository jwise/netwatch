#ifndef __IO_H
#define __IO_H

#define inb(port) __extension__ ({ unsigned char val; asm volatile("inb %%dx, %0" : "=a" ((unsigned char)(val)) : "d" ((unsigned short)(port))); val; })
#define inw(port) __extension__ ({ unsigned short val; asm volatile("inw %%dx, %0" : "=a" ((unsigned short)(val)) : "d" ((unsigned short)(port))); val; })
#define inl(port) __extension__ ({ unsigned long val; asm volatile("inl %%dx, %0" : "=a" ((unsigned long)(val)) : "d" ((unsigned short)(port))); val; })
#define outb(port, val) __extension__ ({ asm volatile("outb %0, %%dx" : : "a" ((unsigned char)(val)) , "d" ((unsigned short)(port))); })
#define outw(port, val) __extension__ ({ asm volatile("outw %0, %%dx" : : "a" ((unsigned short)(val)) , "d" ((unsigned short)(port))); })
#define outl(port, val) __extension__ ({ asm volatile("outl %0, %%dx" : : "a" ((unsigned long)(val)) , "d" ((unsigned short)(port))); })

#endif
