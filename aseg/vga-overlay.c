#include <io.h>
#include <smram.h>
#include <video_defines.h>
#include <minilib.h>
#include <stdarg.h>

static char logents[4][41] = {{0}};

#define VRAM_BASE		0xA0000UL
#define TEXT_CONSOLE_OFFSET	0x18000UL 

#define TEXT_CONSOLE_BASE	(VRAM_BASE + TEXT_CONSOLE_OFFSET)

#define COLOR			0x1F

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

void strblit(char *src, int row, int col)
{
	char *destp = vga_base() + row * 80 * 2 + col * 2;
	smram_state_t old_state = smram_save_state();

	smram_aseg_set_state(SMRAM_ASEG_SMMCODE);
	
	while (*src)
	{
		*(destp++) = *(src++);
		*(destp++) = COLOR;
	}

	smram_restore_state(old_state);
}

void outlog()
{
	int y, x;
	char *basep = vga_base();

	smram_state_t old_state = smram_save_state();

	smram_aseg_set_state(SMRAM_ASEG_SMMCODE);

	for (y = 0; y < 4; y++)
		for (x = 40; x < 80; x++)
		{
			basep[y*80*2+x*2] = ' ';
			basep[y*80*2+x*2+1] = 0x1F;
		}

	smram_restore_state(old_state);
	
	for (y = 0; y < 4; y++)
		strblit(logents[y], y, 40);
}

void dolog(char *s)
{
	memmove(logents[0], logents[1], sizeof(logents[0])*3);
	strcpy(logents[3], s);
}

void dologf(char *fmt, ...)
{
	va_list va;
	
	memmove(logents[0], logents[1], sizeof(logents[0])*3);
	va_start(va, fmt);
	vsnprintf(logents[3], 40, fmt, va);
	va_end(va);
}
