#ifndef __FB_H
#define __FB_H

struct fbdevice;
struct vmode;

typedef enum {
	FB_RGB888
} format_t;

typedef void (*getvmode_t)(void *);

struct vmode {
	int text:1;
	int xres, yres, bytestride;
	format_t format;
};

struct fbdevice {
	unsigned char *fbaddr;
	void *priv;
	getvmode_t getvmode;
	struct vmode curmode;
};

extern struct fbdevice *fb;

#endif
