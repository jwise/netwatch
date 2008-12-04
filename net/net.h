#ifndef _NET_H
#define _NET_H

#include "etherboot-compat.h"

extern void eth_init();
extern void eth_poll();
extern int eth_register(struct nic *nic);

#endif
