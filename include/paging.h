#ifndef __PAGING_H
#define __PAGING_H

extern unsigned long v2p(void *virt);
extern void *p2v(unsigned long phys);
extern int addmap(unsigned long vaddr, unsigned long paddr);
extern void *demap(unsigned long _pd, unsigned long vaddr);

#endif
