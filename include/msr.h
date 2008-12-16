/* msr.h
 * Macros to read and write model specific registers
 * NetWatch system management mode administration console
 *
 * Copyright (c) 2008 Jacob Potter and Joshua Wise.  All rights reserved.
 * This program is free software; you can redistribute and/or modify it under
 * the terms found in the file LICENSE in the root of this source tree. 
 *
 */

#ifndef _MSR_H
#define _MSR_H

#define WRMSR(ad, da) \
	do { \
		unsigned long __a = (ad); \
		unsigned long long __d = (da); \
		asm volatile("wrmsr" : : "c" (__a), "A" (__d)); \
	} while (0)
#define RDMSR(ad) \
	({ \
		unsigned long __a = (ad); \
		unsigned long long __d; \
		asm volatile("rdmsr" : "=A" (__d) : "c" (__a)); \
		__d; \
	})

#endif
