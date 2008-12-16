/* crc32.h
 * CRC32 routine declarations
 * NetWatch system management mode administration console
 *
 * Copyright (c) 2008 Jacob Potter and Joshua Wise.  All rights reserved.
 * This program is free software; you can redistribute and/or modify it under
 * the terms found in the file LICENSE in the root of this source tree. 
 *
 */

#ifndef _CRC32_H
#define _CRC32_H

#include <stdint.h>

extern uint32_t crc32(unsigned char *buf, int len, uint32_t crc0);
extern void crc32_init();

#endif

