/* cs410.c
 * Entry points for 15-410 extensions
 * NetWatch system management mode administration console
 *
 * Copyright (c) 2008 Jacob Potter and Joshua Wise.  All rights reserved.
 * This program is free software; you can redistribute and/or modify it under
 * the terms found in the file LICENSE in the root of this source tree.
 *
 */

#include <io.h>
#include <smi.h>
#include <smram.h>
#include <video_defines.h>
#include <output.h>
#include <minilib.h>

#define CONSCOLOR 0x1F

unsigned char oldcons[80 * 25 * 2];

static unsigned char vga_read(unsigned char idx)
{
	outb(CRTC_IDX_REG, idx);
	return inb(CRTC_DATA_REG);
}

static char * vga_base()
{
	return (char *) (
		0xB8000
		+ (((unsigned int) vga_read(CRTC_START_ADDR_MSB_IDX)) << 9)
		+ (((unsigned int) vga_read(CRTC_START_ADDR_LSB_IDX)) << 1)
	);
}

static void grey_console()
{
	int i;
	char *p = vga_base();
	
	for (i = 0; i < 80*25; i++, p+=2)
		p[1] &= ~0xF8;	/* Clear the background and any brightness. */
}

#define CHAR(x, y) (base[((y)*80+(x))*2])
#define COLOR(x, y) (base[((y)*80+(x))*2+1])

static void ui_frame(int xp, int yp, int xs, int ys, char *title)
{
	int x, y;
	char *base = vga_base();
	
	/* Blank and fill the region. */
	for (y = yp; y <= (yp + ys); y++)
		for (x = xp; x <= (xp + xs); x++)
		{
			CHAR(x, y) = ' ';
			COLOR(x, y) = CONSCOLOR;
		}

	/* Draw the top and the bottom (and the bar if need be). */	
	for (x = xp; x <= (xp + xs); x++)
	{
		CHAR(x, yp) = 0xCD /* double line horizontal */;
		CHAR(x, yp+ys) = 0xCD /* double line horizontal */;
		if (title)
			CHAR(x, yp+2) = 0xC4 /* single line horizontal */;
	}
	
	/* Draw the left and right. */
	for (y = yp; y <= (yp + ys); y++)
	{
		CHAR(xp, y) = 0xBA /* double line vertical */;
		CHAR(xp+xs, y) = 0xBA /* double line vertical */;
	}
	
	/* Drop in the corners. */
	CHAR(xp, yp) = 0xC9;
	CHAR(xp+xs, yp) = 0xBB;
	CHAR(xp, yp+ys) = 0xC8;
	CHAR(xp+xs, yp+ys) = 0xBC;
	if (title)
	{
		CHAR(xp, yp+2) = 0xC7;
		CHAR(xp+xs, yp+2) = 0xB6;
	}
	
	/* Drop in the text. */
	if (title)
		for (x = (xs - strlen(title)) / 2 + xp; *title; x++, title++)
			CHAR(x, yp+1) = *title;
}

static void ui_label(int xp, int yp, char *s)
{
	int x;
	char *base = vga_base();
	
	for (x = xp; *s; x++, s++)
		CHAR(x, yp) = *s;
}

typedef enum ui_element_type {
	UI_FRAME = 0,
	UI_LABEL = 1,
	UI_END = -1
} ui_element_type_t;

typedef union ui_element {
	ui_element_type_t type;
	struct {
		ui_element_type_t type;
		int xp, yp;
		int xs, ys;
		char *title;
	} frame;
	struct {
		ui_element_type_t type;
		int xp, yp;
		char *text;
	} label;
} ui_element_t;

static ui_element_t mainmenu[] = {
	{ .frame = {
		.type = UI_FRAME,
		.xp = 10, .yp = 5,
		.xs = 60, .ys = 15,
		.title = "NetWatch main menu" } },
	{ .label = {
		.type = UI_LABEL,
		.xp = 12, .yp = 8,
		.text = "Options:" } },
	{ .label = {
		.type = UI_LABEL,
		.xp = 12, .yp = 10,
		.text = "q - Return to system" } },
	{ .type = UI_END }
};

static void ui_render(ui_element_t *ui)
{
	for (; ui->type != UI_END; ui++)
		switch (ui->type)
		{
		case UI_FRAME:
			ui_frame(ui->frame.xp, ui->frame.yp,
			         ui->frame.xs, ui->frame.ys,
			         ui->frame.title);
			break;
		case UI_LABEL:
			ui_label(ui->label.xp, ui->label.yp,
			         ui->label.text);
			break;
		case UI_END:
		default:
			return;	/* ??? */
		}
}

void cs410_pwrbtn_handler(smi_event_t ev)
{
	smram_state_t old_state;
	
	outputf("410watch: Power button event");
	
	/* Save off the console. */
	old_state = smram_save_state();
	smram_aseg_set_state(SMRAM_ASEG_SMMCODE);
	memcpy(oldcons, vga_base(), 80*25*2);
	
	/* Pull a Simics, and grey out the console -- why not? */
	grey_console();
	
	/* Show the main menu. */
	ui_render(mainmenu);
	
	/* Allow access to data in ASEG. */
	smram_restore_state(old_state);
	
	/* Now just sit for a while to show off our newly greyed console. */
	char c;
	char *p = vga_base() + (11*80+12)*2;
	extern int getchar();
	while ((c = getchar()) != 'q')
	{
		smram_aseg_set_state(SMRAM_ASEG_SMMCODE);
		*p = c;
		p += 2;
		smram_restore_state(old_state);
	}
	
	/* Put the console back. */
	smram_aseg_set_state(SMRAM_ASEG_SMMCODE);
	memcpy(vga_base(), oldcons, 80*25*2);
	smram_restore_state(old_state);

}
