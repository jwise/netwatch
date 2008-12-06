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
	void (*transmit) (struct pbuf *p);
};

#define virt_to_bus(x) memory_v2p((void *)(x))

#endif
