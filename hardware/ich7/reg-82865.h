/* reg-82865.h
 * SMRAM register definitions for Intel 82865 northbridge
 * NetWatch system management mode administration console
 *
 * Copyright (c) 2008 Jacob Potter and Joshua Wise.  All rights reserved.
 * This program is free software; you can redistribute and/or modify it under
 * the terms found in the file LICENSE in the root of this source tree. 
 *
 */

#ifndef _REG_82865_H
#define _REG_82865_H

#define SMRAMC 0x9D
#define SMRAMC_ENABLE 0x8
#define SMRAMC_LOCK 0x10
#define SMRAMC_CLOSE 0x20
#define SMRAMC_OPEN 0x40
#define ESMRAMC 0x9E
#define ESMRAMC_TSEGEN 0x1
#define ESMRAMC_TSEGSZ 0x6
#define ESMRAMC_SMERR 0x40
#define ESMRAMC_HISMRAM 0x80


#endif
