/* output.h
 * Register macros for Intel 82815
 * NetWatch system management mode administration console
 *
 * Copyright (c) 2008 Jacob Potter and Joshua Wise.  All rights reserved.
 * This program is free software; you can redistribute and/or modify it under
 * the terms found in the file LICENSE in the root of this source tree.
 *
 */

#ifndef _REG_82815_H
#define _REG_82815_H

#define DRP 0x52
#define DRP2 0x54

#define SMRAMC 0x70
#define SMRAMC_LOCK 0x02
#define SMRAMC_LSMM 0x0C
#define SMRAMC_USMM 0x30
#define SMRAMC_GMS  0xC0

#endif
