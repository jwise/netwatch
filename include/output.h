/* output.h
 * Definitions for needed output routines
 * NetWatch system management mode administration console
 *
 * Copyright (c) 2008 Jacob Potter and Joshua Wise.  All rights reserved.
 * This program is free software; you can redistribute and/or modify it under
 * the terms found in the file LICENSE in the root of this source tree. 
 *
 */

#ifndef __OUTPUT_H
#define __OUTPUT_H

extern void (*output)(const char *s);
extern void (*outputf)(const char *s, ...);

#endif
