#ifndef _PAGETABLE_H
#define _PAGETABLE_H

#include <stdint.h>

void * l2p(void * addr);

typedef struct {
	uint32_t signature;
	uint32_t type;
	uint8_t data[];
} packet_t;

packet_t * check_packet(uint32_t logical_addr);

#endif /* _PAGETABLE_H */
