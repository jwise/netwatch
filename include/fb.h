/* fb.h
 * Framebuffer definitions
 * NetWatch system management mode administration console
 *
 * Copyright (c) 2008 Jacob Potter and Joshua Wise.  All rights reserved.
 * This program is free software; you can redistribute and/or modify it under
 * the terms found in the file LICENSE in the root of this source tree. 
 *
 */

#ifndef __FB_H
#define __FB_H

#include <stdint.h>

struct fbdevice;
struct vmode;

typedef enum {
	FB_RGB888
} format_t;

typedef void (*getvmode_t)(void *);
typedef uint32_t (*checksum_rect_t)(int x, int y, int width, int height);
typedef void (*copy_pixels_t)(char *buf, int x, int y, int width, int height);

struct vmode {
	int text:1;
	int xres, yres, bytestride;
	format_t format;
};

struct fbdevice {
	unsigned char *fbaddr;
	unsigned char *textbase;	/* A safe place to put a textfb. */
	void *priv;
	getvmode_t getvmode;
	checksum_rect_t checksum_rect;
	copy_pixels_t copy_pixels;
	struct vmode curmode;
};

extern struct fbdevice *fb;

#endif
