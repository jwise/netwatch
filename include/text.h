/* text.h
 * Prototypes for text rendering routines
 * NetWatch system management mode administration console
 *
 * Copyright (c) 2008 Jacob Potter and Joshua Wise.  All rights reserved.
 * This program is free software; you can redistribute and/or modify it under
 * the terms found in the file LICENSE in the root of this source tree.
 *
 */

#ifndef _TEXT_H

#include <stdint.h>

extern void text_init();
extern void text_render(char *buf, int x, int y, int w, int h);
extern uint32_t text_checksum(int x, int y, int w, int h);

#endif
