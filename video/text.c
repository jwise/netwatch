#include <io.h>
#include <text.h>
#include <paging.h>
#include <minilib.h>
#include <stdint.h>
#include <output.h>
#include <smram.h>

static unsigned char _font[256 * 32];

/* Must be called from a firstrun context, where we don't care about saving
 * 0x3CE state. */
void text_init()
{
	unsigned char oldread;
	smram_state_t old_state = smram_save_state();
	outb(0x3CE, 0x05 /* Mode register */);
	outb(0x3CF, inb(0x3CF) & ~(0x10 /* Odd/even */));
	outb(0x3CE, 0x04 /* Read register */);
	oldread = inb(0x3CF);
	outb(0x3CF, 0x02 /* Font plane */);
	smram_aseg_set_state(SMRAM_ASEG_SMMCODE);
	memcpy(_font, p2v(0xB8000), sizeof(_font));
	smram_restore_state(old_state);
	outb(0x3CF, oldread);
}

void text_render(char *buf, int x, int y, int w, int h)
{
	unsigned char *video = (unsigned char *)0xB8000;
	unsigned int textx = x / 9;
	unsigned int texty = y / 14;
	unsigned int cx, cy;
	unsigned char ch, at, font;
	smram_state_t old_state = smram_save_state();
	
	outputf("text_render: (%d,%d),(%d,%d)", buf, x, y, w, h);
	
	smram_aseg_set_state(SMRAM_ASEG_SMMCODE);
	for (cy = y; cy < (y + h); cy++)
	{
		cx = x;
		texty = cy / 14;
		textx = cx / 9;
		ch = video[texty * 50 + textx * 2];
		at = video[texty * 50 + textx * 2 + 1];
		font = _font[ch * 32 + cy % 14];
		for (cx = x; cx < (x + w); cx++)
		{
			unsigned int pos = cx % 9;
			if (pos == 0)
			{
				textx = cx / 9;
				ch = video[texty * 50 + textx * 2];
				at = video[texty * 50 + textx * 2 + 1];
				font = _font[ch * 32 + cy % 14];
			}
			/* XXX always BGR888 */
			if (pos == 8)	/* 9th pixel is cloned */
				pos = 7;
			if ((font >> (7 - pos)) & 1)
			{
				*(buf++) = (at & 0x01) ? 0xFF : 0x00;
				*(buf++) = (at & 0x02) ? 0xFF : 0x00;
				*(buf++) = (at & 0x04) ? 0xFF : 0x00;
			} else {
				*(buf++) = (at & 0x10) ? 0xFF : 0x00;
				*(buf++) = (at & 0x20) ? 0xFF : 0x00;
				*(buf++) = (at & 0x40) ? 0xFF : 0x00;
			}
			*(buf++) = 0;
		}
	}
	smram_restore_state(old_state);
}

uint32_t text_checksum(int x, int y, int w, int h)
{
	unsigned char *video = (unsigned char *)0xB8000;
	unsigned int textx = x / 9;
	unsigned int texty = y / 14;
	int cx, cy;
	unsigned char ch, at;
	uint32_t cksm = 0;
	smram_state_t old_state = smram_save_state();
	
	smram_aseg_set_state(SMRAM_ASEG_SMMCODE);
	
	for (cy = y; cy < (y + h); cy++)
	{
		cx = x;
		texty = cy / 14;
		textx = cx / 9;
		ch = video[texty * 50 + textx * 2];
		at = video[texty * 50 + textx * 2 + 1];
		for (cx = x; cx < (x + w); cx++)
		{
			unsigned int pos = cx % 9;
			if (pos == 0)
			{
				textx = cx / 9;
				ch = video[texty * 50 + textx * 2];
				at = video[texty * 50 + textx * 2 + 1];
			}
			
			cksm += ch + (at << 16);
		}
	}
	
	smram_restore_state(old_state);
	
	return cksm;
}
