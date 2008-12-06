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
