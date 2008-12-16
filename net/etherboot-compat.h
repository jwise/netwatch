/* etherboot-compat.h
 * EtherBoot driver compatibility routines
 * NetWatch system management mode administration console
 *
 * Copyright (c) 2008 Jacob Potter and Joshua Wise.  All rights reserved.
 * This program is free software; you can redistribute and/or modify it under
 * the terms found in the file LICENSE in the root of this source tree.
 *
 */


#ifndef _ETHERBOOT_COMPAT_H
#define _ETHERBOOT_COMPAT_H

#include <paging.h>
#include "lwip/pbuf.h"

#define ETH_ALEN	6

struct dev {
	void (*disable) (struct dev *dev);
};

struct nic {
	char * packet;
	int packetlen;

	unsigned int ioaddr;
	unsigned int irqno;
	
	unsigned char hwaddr[6];

	int (*recv) (struct nic *nic);
	void (*transmit) (struct nic *nic, struct pbuf *p);
};

#define virt_to_bus(x) memory_v2p((void *)(x))

#endif
