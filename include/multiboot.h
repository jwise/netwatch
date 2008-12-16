/* minilib.h
 * Definitions needed for a multiboot kernel
 * NetWatch system management mode administration console
 *
 * Copyright (c) 2008 Jacob Potter and Joshua Wise.  All rights reserved.
 * This program is free software; you can redistribute and/or modify it under
 * the terms found in the file LICENSE in the root of this source tree. 
 *
 */

#ifndef __MULTIBOOT_H
#define __MULTIBOOT_H

#define MULTIBOOT_LOADER_MAGIC 0x2BADB002

struct mb_info
{
	unsigned long flags;
	unsigned long mem_lower, mem_upper;
	unsigned long boot_dev;
	char *cmdline;
	unsigned long mod_cnt;
	struct mod_info *mods;
};

struct mod_info
{
	void *mod_start;
	void *mod_end;
	char *mod_string;
	void *reserved;
};

#endif
