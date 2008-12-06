#ifndef _NET_H
#define _NET_H

#include "etherboot-compat.h"
#include <lwip/pbuf.h>

extern void eth_init();
extern void eth_recv(struct nic *nic, struct pbuf *p);
extern void eth_poll();
extern int eth_register(struct nic *nic);

#endif
