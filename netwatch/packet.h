/* packet.h
 * OS-to-NetWatch communication packet interface
 * NetWatch system management mode administration console
 *
 * Copyright (c) 2008 Jacob Potter and Joshua Wise.  All rights reserved.
 * This program is free software; you can redistribute and/or modify it under
 * the terms found in the file LICENSE in the root of this source tree.
 *
 */

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
