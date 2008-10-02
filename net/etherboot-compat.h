#ifndef _ETHERBOOT_COMPAT_H
#define _ETHERBOOT_COMPAT_H

#define ETH_ALEN	6

struct dev {
	void (*disable) (struct dev *dev);
};

struct nic {
	char * packet;
	int packetlen;

	unsigned int ioaddr;
	unsigned int irqno;

	int (*poll) (struct nic *nic, int retrieve);
	void (*transmit) (const char *dest_addr, unsigned int proto, unsigned int size, const char *pkt);
};

#define virt_to_bus(x) ((unsigned long)x)

#endif
