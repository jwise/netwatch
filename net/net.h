#ifndef _3C905_H
#define _3C905_H

#include "etherboot-compat.h"

extern void eth_init();
extern void eth_poll();
extern int eth_register(struct nic *nic);

#endif
