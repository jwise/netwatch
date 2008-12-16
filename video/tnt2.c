/* tnt2.c
 * NVidia TNT2 driver.
 * NetWatch system management mode administration console
 *
 * Copyright (c) 2008 Jacob Potter and Joshua Wise.  All rights reserved.
 * This program is free software; you can redistribute and/or modify it under
 * the terms found in the file LICENSE in the root of this source tree.
 *
 */

#include <minilib.h>
#include <io.h>
#include <pci.h>
#include <output.h>
#include <fb.h>
#include <paging.h>
#include <text.h>

#include "generic.h"

static void tnt2_getvmode(void *priv);

static struct fbdevice tnt2_fb = {
	.getvmode = &tnt2_getvmode,
};

static unsigned int vgard(unsigned char a)
{
	outb(0x3D4, a);
	return (unsigned int)inb(0x3D5);
}

static void tnt2_getvmode(void *priv)
{
	tnt2_fb.curmode.xres = (vgard(0x1) + 1) * 8;
	tnt2_fb.curmode.yres = (vgard(0x12) | (vgard(0x7) & 2) << 7 | (vgard(0x7) & 0x40) << 3) + 1;
	switch (vgard(0x28))
	{
	case 3:
		tnt2_fb.curmode.format = FB_RGB888;
		tnt2_fb.curmode.bytestride = 4;
		tnt2_fb.curmode.text = 0;
		tnt2_fb.checksum_rect = checksum_rect_generic32;
		tnt2_fb.copy_pixels = copy_pixels_generic32;
		break;
	case 0:
		tnt2_fb.curmode.text = 1;
		tnt2_fb.checksum_rect = text_checksum;
		tnt2_fb.copy_pixels = text_render;
		break;
	default:
		tnt2_fb.curmode.text = 1;
		tnt2_fb.checksum_rect = text_checksum;
		tnt2_fb.copy_pixels = text_render;
		outputf("Unknown TNT2 format %x", vgard(0x28));
		break;
	}
}

static int tnt2_probe(struct pci_dev *pci, void *data)
{
	unsigned int p;

	if (pci->bars[1].type != PCI_BAR_MEMORY32)
	{
		output("TNT2 BAR1 is not memory32?");
		return 0;
	}
	
	/* Map 32M of memory. */
	for (p = 0; p < 32; p += 4)
		addmap_4m(0x40000000 + p*1024*1024, pci->bars[1].addr + p*1024*1024);
	tnt2_fb.fbaddr = (void *)0x40000000;
	
	fb = &tnt2_fb;
	outputf("Found TNT2 with FB at %08x, mapped to %08x", pci->bars[1].addr, tnt2_fb.fbaddr);
	return 1;
}

static struct pci_id tnt2_pci[] = {
	{0x10DE, 0x0028, "TNT2", "RIVA TNT2"}
};

struct pci_driver tnt2_driver = {
	.name     = "tnt2",
	.probe    = tnt2_probe,
	.ids      = tnt2_pci,
	.id_count = sizeof(tnt2_pci)/sizeof(tnt2_pci[0]),
};
