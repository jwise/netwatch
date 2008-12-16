/* stdint.h
 * Standard integer types
 * NetWatch system management mode administration console
 *
 * Copyright (c) 2008 Jacob Potter and Joshua Wise.  All rights reserved.
 * This program is free software; you can redistribute and/or modify it under
 * the terms found in the file LICENSE in the root of this source tree.
 *
 */

#ifndef _STDINT_H
#define _STDINT_H

#ifndef __i386__
# ifndef __x86_64__
#  error this stdint.h expects either __i386__ or __x86_64__ to be defined
# endif
#endif

typedef signed char	int8_t;
typedef unsigned char	uint8_t;

typedef signed short	int16_t;
typedef unsigned short	uint16_t;

typedef signed int	int32_t;
typedef unsigned int	uint32_t;

#ifdef __x86_64__
typedef signed long	int64_t;
typedef unsigned long	uint64_t;
#else
typedef signed long long	int64_t;
typedef unsigned long long	uint64_t;
#endif

#endif
