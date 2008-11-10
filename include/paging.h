#ifndef __PAGING_H
#define __PAGING_H

extern unsigned long memory_v2p(void *virt);
extern void *memory_p2v(unsigned long phys);

#endif
