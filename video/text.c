#include <io.h>
#include <text.h>
#include <paging.h>
#include <minilib.h>
#include <stdint.h>
#include <output.h>
#include <smram.h>
#include <crc32.h>
#include <video_defines.h>

static unsigned char _font[256 * 32];

/* XXX reunify this with vga-overlay? */
#define VRAM_BASE		0xA0000UL
#define TEXT_CONSOLE_OFFSET	0x18000UL 

#define TEXT_CONSOLE_BASE	(VRAM_BASE + TEXT_CONSOLE_OFFSET)

static unsigned char vga_read(unsigned char idx)
{
	outb(CRTC_IDX_REG, idx);
	return inb(CRTC_DATA_REG);
}

static char * vga_base()
{
	return (char *) (
		TEXT_CONSOLE_BASE
		+ (((unsigned int) vga_read(CRTC_START_ADDR_MSB_IDX)) << 9)
		+ (((unsigned int) vga_read(CRTC_START_ADDR_LSB_IDX)) << 1)
	);
}

/* Must be called from a firstrun context, where we don't care about saving
 * 0x3CE state. */
void text_init()
{
	unsigned char oldread;

	smram_state_t old_state = smram_save_state();
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
	unsigned char *video = (unsigned char *)vga_base();
	unsigned int textx = x / 9;
	unsigned int texty = y / 14;
	unsigned int cx, cy;
	unsigned char ch, at, font;
	smram_state_t old_state = smram_save_state();
	
	outputf("text_render: (%d,%d),(%d,%d)", x, y, w, h);
	
	smram_aseg_set_state(SMRAM_ASEG_SMMCODE);
	for (cy = y; cy < (y + h); cy++)
	{
		cx = x;
		texty = cy / 14;
		textx = cx / 9;
		ch = video[texty * 160 + textx * 2];
		at = video[texty * 160 + textx * 2 + 1];
		font = _font[ch * 32 + cy % 14];
		for (cx = x; cx < (x + w); cx++)
		{
			unsigned int pos = cx % 9;
			if (pos == 0)
			{
				textx = cx / 9;
				ch = video[texty * 160 + textx * 2];
				at = video[texty * 160 + textx * 2 + 1];
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
	unsigned char *video = (unsigned char *)vga_base();
	unsigned int textx = x / 9;
	unsigned int texty = y / 14;
	int cx, cy;
	uint32_t cksm = 0;
	smram_state_t old_state = smram_save_state();
	
	smram_aseg_set_state(SMRAM_ASEG_SMMCODE);
	
	for (cy = y; cy < (y + h); cy++)
	{
		cx = x;
		texty = cy / 14;
		textx = cx / 9;
		cksm = crc32(video + texty * 160 + textx * 2, (w / 9) * 2 + 2, cksm);	/* Err on the side of 'too many'. */
	}
	
	smram_restore_state(old_state);
	
	return cksm;
}
