/* serial.h
 * Prototypes for serial output routines
 * NetWatch system management mode administration console
 *
 * Copyright (c) 2008 Jacob Potter and Joshua Wise.  All rights reserved.
 * This program is free software; you can redistribute and/or modify it under
 * the terms found in the file LICENSE in the root of this source tree.
 *
 */

#ifndef SERIAL_H
#define SERIAL_H

extern void serial_init();
extern void serial_tx(unsigned char c);

#endif
