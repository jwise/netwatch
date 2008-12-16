/* minilib.h
 * Definitions for page-table mapping and demapping
 * NetWatch system management mode administration console
 *
 * Copyright (c) 2008 Jacob Potter and Joshua Wise.  All rights reserved.
 * This program is free software; you can redistribute and/or modify it under
 * the terms found in the file LICENSE in the root of this source tree. 
 *
 */

#ifndef __PAGING_H
#define __PAGING_H

extern unsigned long v2p(void *virt);
extern void *p2v(unsigned long phys);
extern int addmap(unsigned long vaddr, unsigned long paddr);
extern int addmap_4m(unsigned long vaddr, unsigned long paddr);
extern void *demap(unsigned long _pd, unsigned long vaddr);

#endif
