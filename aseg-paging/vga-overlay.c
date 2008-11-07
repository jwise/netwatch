#include <io.h>
#include <smram.h>
#include <video_defines.h>
#include <minilib.h>
#include <stdarg.h>
#include <output.h>
#include <serial.h>

#define LOGLEN 96
#define LOG_ONSCREEN 4

static char logents[LOGLEN][41] = {{0}};
static int prodptr = 0;
static int flush_imm = 0;

#define VRAM_BASE		0xA0000UL
#define TEXT_CONSOLE_OFFSET	0x18000UL 

#define TEXT_CONSOLE_BASE	(VRAM_BASE + TEXT_CONSOLE_OFFSET)

#define COLOR			0x1F

void vga_flush_imm(int imm)
{
	flush_imm = imm;
}

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
	outb(0x80, 0x3C);
	smram_state_t old_state = smram_save_state();
	outb(0x80, 0x3D);

	smram_aseg_set_state(SMRAM_ASEG_SMMCODE);
	outb(0x80, 0x3E);

	while (*src)
	{
		*(destp++) = *(src++);
		*(destp++) = COLOR;
	}

	outb(0x80, 0x3F);
	smram_restore_state(old_state);
	outb(0x80, 0x40);
}

void outlog()
{
	int y, x;
	char *basep = vga_base();

	smram_state_t old_state = smram_save_state();

	smram_aseg_set_state(SMRAM_ASEG_SMMCODE);

	for (y = 0; y < LOG_ONSCREEN; y++)
		for (x = 40; x < 80; x++)
		{
			basep[y*80*2+x*2] = ' ';
			basep[y*80*2+x*2+1] = 0x1F;
		}

	smram_restore_state(old_state);
	
	for (y = -LOG_ONSCREEN; y < 0; y++)
		strblit(logents[(y + prodptr) % LOGLEN], y + LOG_ONSCREEN, 40);
}

void dolog(const char *s)
{
	strcpy(logents[prodptr], s);
	prodptr = (prodptr + 1) % LOGLEN;
	while (*s)
		serial_tx(*s);
	if (flush_imm)
		outlog();
}
void (*output)(const char *s) = dolog;

void dologf(const char *fmt, ...)
{
}
void (*outputf)(const char *s, ...) = dologf;

void dump_log (char * target) {
	memcpy(target, logents, sizeof(logents));
}

