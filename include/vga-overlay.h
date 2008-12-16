/* vga-overlay.h
 * Prototype for VGA text overlay routines
 * NetWatch system management mode administration console
 *
 * Copyright (c) 2008 Jacob Potter and Joshua Wise.  All rights reserved.
 * This program is free software; you can redistribute and/or modify it under
 * the terms found in the file LICENSE in the root of this source tree.
 *
 */

#ifndef VGA_OVERLAY_H
#define VGA_OVERLAY_H

void vga_flush_imm(int enb);

void strblit(char *src, int row, int col, int fill);

void dolog(char *s);
void dologf(char *s, ...);
void outlog();

void dump_log(char *buffer);

#endif
