/* net.h
 * Top-level network clue code headers
 * NetWatch system management mode administration console
 *
 * Copyright (c) 2008 Jacob Potter and Joshua Wise.  All rights reserved.
 * This program is free software; you can redistribute and/or modify it under
 * the terms found in the file LICENSE in the root of this source tree.
 *
 */

#ifndef _NET_H
#define _NET_H

#include "etherboot-compat.h"
#include <lwip/pbuf.h>

extern void eth_init();
extern void eth_recv(struct nic *nic, struct pbuf *p);
extern void eth_poll();
extern int eth_register(struct nic *nic);

#endif
