#include "keyboard.h"
#include <stdint.h>
#include <minilib.h>
#include <output.h>

static unsigned char kbd_inj_buffer[128];
static int kbd_inj_start = 0;
static int kbd_inj_end = 0;
int kbd_mode = 1;

static const char scancodes2[][2][8] = {
	['a'] = { "\x1c", "\xf0\x1c" },
	['b'] = { "\x32", "\xf0\x32" },
	['c'] = { "\x21", "\xf0\x21" },
	['d'] = { "\x23", "\xf0\x23" },
	['e'] = { "\x24", "\xf0\x24" },
	['f'] = { "\x2b", "\xf0\x2b" },
	['g'] = { "\x34", "\xf0\x34" },
	['h'] = { "\x33", "\xf0\x33" },
	['i'] = { "\x43", "\xf0\x43" }, 
	['j'] = { "\x3b", "\xf0\x3b" },
	['k'] = { "\x42", "\xf0\x42" },
	['l'] = { "\x4b", "\xf0\x4b" },
	['m'] = { "\x3a", "\xf0\x3a" },
	['n'] = { "\x31", "\xf0\x31" },
	['o'] = { "\x44", "\xf0\x44" },
	['p'] = { "\x4d", "\xf0\x4d" },
	['q'] = { "\x15", "\xf0\x15" },
	['r'] = { "\x2d", "\xf0\x2d" },
	['s'] = { "\x1b", "\xf0\x1b" },
	['t'] = { "\x2c", "\xf0\x2c" },
	['u'] = { "\x3c", "\xf0\x3c" },
	['v'] = { "\x2a", "\xf0\x2a" },
	['w'] = { "\x1d", "\xf0\x1d" },
	['x'] = { "\x22", "\xf0\x22" },
	['y'] = { "\x35", "\xf0\x35" },
	['z'] = { "\x1a", "\xf0\x1a" },
	['A'] = { "\x12\x1c", "\xf0\x1c\xf0\x12" },
	['B'] = { "\x12\x32", "\xf0\x32\xf0\x12" },
	['C'] = { "\x12\x21", "\xf0\x21\xf0\x12" },
	['D'] = { "\x12\x23", "\xf0\x23\xf0\x12" },
	['E'] = { "\x12\x24", "\xf0\x24\xf0\x12" },
	['F'] = { "\x12\x2b", "\xf0\x2b\xf0\x12" },
	['G'] = { "\x12\x34", "\xf0\x34\xf0\x12" },
	['H'] = { "\x12\x33", "\xf0\x33\xf0\x12" },
	['I'] = { "\x12\x43", "\xf0\x43\xf0\x12" },
	['J'] = { "\x12\x3b", "\xf0\x3b\xf0\x12" },
	['K'] = { "\x12\x42", "\xf0\x42\xf0\x12" },
	['L'] = { "\x12\x4b", "\xf0\x4b\xf0\x12" },
	['M'] = { "\x12\x3a", "\xf0\x3a\xf0\x12" },
	['N'] = { "\x12\x31", "\xf0\x31\xf0\x12" },
	['O'] = { "\x12\x44", "\xf0\x44\xf0\x12" },
	['P'] = { "\x12\x4d", "\xf0\x4d\xf0\x12" },
	['Q'] = { "\x12\x15", "\xf0\x15\xf0\x12" },
	['R'] = { "\x12\x2d", "\xf0\x2d\xf0\x12" },
	['S'] = { "\x12\x1b", "\xf0\x1b\xf0\x12" },
	['T'] = { "\x12\x2c", "\xf0\x2c\xf0\x12" },
	['U'] = { "\x12\x3c", "\xf0\x3c\xf0\x12" },
	['V'] = { "\x12\x2a", "\xf0\x2a\xf0\x12" },
	['W'] = { "\x12\x1d", "\xf0\x1d\xf0\x12" },
	['X'] = { "\x12\x22", "\xf0\x22\xf0\x12" },
	['Y'] = { "\x12\x35", "\xf0\x35\xf0\x12" },
	['Z'] = { "\x12\x1a", "\xf0\x1a\xf0\x12" },
	['`'] = { "\x0e", "\xf0\x0e" },
	['~'] = { "\x12\x0e", "\xf0\x0e\xf0\x12" },
	['1'] = { "\x16", "\xf0\x16" },
	['!'] = { "\x12\x16", "\xf0\x16\xf0\x12" },
	['2'] = { "\x1e", "\xf0\x1e" },
	['@'] = { "\x12\x1e", "\xf0\x1e\xf0\x12" },
	['3'] = { "\x26", "\xf0\x26" },
	['#'] = { "\x12\x26", "\xf0\x26\xf0\x12" },
	['4'] = { "\x25", "\xf0\x25" },
	['$'] = { "\x12\x25", "\xf0\x25\xf0\x12" },
	['5'] = { "\x2e", "\xf0\x2e" },
	['%'] = { "\x12\x2e", "\xf0\x2e\xf0\x12" },
	['6'] = { "\x36", "\xf0\x36" },
	['^'] = { "\x12\x36", "\xf0\x36\xf0\x12" },
	['7'] = { "\x3d", "\xf0\x3d" },
	['&'] = { "\x12\x3d", "\xf0\x3d\xf0\x12" },
	['8'] = { "\x3e", "\xf0\x3e" },
	['*'] = { "\x12\x3e", "\xf0\x3e\xf0\x12" },
	['9'] = { "\x46", "\xf0\x46" },
	['('] = { "\x12\x46", "\xf0\x46\xf0\x12" },
	['0'] = { "\x45", "\xf0\x45" },
	[')'] = { "\x12\x45", "\xf0\x45\xf0\x12" },
	['-'] = { "\x4e", "\xf0\x4e" },
	['_'] = { "\x12\x4e", "\xf0\x4e\xf0\x12" },
	['='] = { "\x55", "\xf0\x55" },
	['+'] = { "\x12\x55", "\xf0\x55\xf0\x12" },
	['['] = { "\x54", "\xf0\x54" },
	['{'] = { "\x12\x54", "\xf0\x54\xf0\x12" },
	[']'] = { "\x5b", "\xf0\x5b" },
	['}'] = { "\x12\x5b", "\xf0\x5b\xf0\x12" },
	['\\'] = { "\x5d", "\xf0\x5d" },
	['|'] = { "\x12\x5d", "\xf0\x5d\xf0\x12" },
	[';'] = { "\x4c", "\xf0\x4c" },
	[':'] = { "\x12\x4c", "\xf0\x4c\xf0\x12" },
	['\''] = { "\x52", "\xf0\x52" },
	['"'] = { "\x12\x52", "\xf0\x52\xf0\x12" },
	[','] = { "\x41", "\xf0\x41" },
	['<'] = { "\x12\x41", "\xf0\x41\xf0\x12" },
	['.'] = { "\x49", "\xf0\x49" },
	['>'] = { "\x12\x49", "\xf0\x49\xf0\x12" },
	['/'] = { "\x4a", "\xf0\x4a" },
	[' '] = { "\x29", "\xf0\x29" },
	['?'] = { "\x12\x4a", "\xf0\x4a\xf0\x12" }
};


static const char scancodes2high[][2][8] = {
	[0x08] = { "\x66", "\xf0\x66" },
	[0x09] = { "\x0d", "\xf0\x0d" },
	[0x0d] = { "\x5a", "\xf0\x5a" },
	[0x1b] = { "\x76", "\xf0\x76" },
	[0x63] = { "\xE0\x70", "\xE0\xF0\x70" },
	[0xff] = { "\xE0\x71", "\xE0\xF0\x71" },
	[0x50] = { "\xE0\x6C", "\xE0\xF0\x6C" },
	[0x57] = { "\xE0\x69", "\xE0\xF0\x69" },
	[0x55] = { "\xE0\x75", "\xE0\xF0\x75" },
	[0x56] = { "\xE0\x7A", "\xE0\xF0\x7A" },
        [0x51] = { "\xE0\x74", "\xE0\xF0\x74" },
        [0x52] = { "\xE0\x75", "\xE0\xF0\x75" },
        [0x53] = { "\xE0\x6B", "\xE0\xF0\x6B" },
        [0x54] = { "\xE0\x72", "\xE0\xF0\x72" },
	[0xe1] = { "\x12", "\xf0\x12" },
	[0xe2] = { "\x59", "\xf0\x59" },
	[0xe3] = { "\x14", "\xf0\x14" },
	[0xe4] = { "\xE0\x14", "\xE0\xF0\x14" },
	[0xe9] = { "\x11", "\xf0\x11" },
	[0xea] = { "\xE0\x11", "\xE0\xF0\x11" }
};

const unsigned char convert_table[] = {
	0xff, 0x43, 0x41, 0x3f, 0x3d, 0x3b, 0x3c, 0x58, 0x64, 0x44, 0x42, 0x40, 0x3e, 0x0f, 0x29, 0x59,
	0x65, 0x38, 0x2a, 0x70, 0x1d, 0x10, 0x02, 0x5a, 0x66, 0x71, 0x2c, 0x1f, 0x1e, 0x11, 0x03, 0x5b,
	0x67, 0x2e, 0x2d, 0x20, 0x12, 0x05, 0x04, 0x5c, 0x68, 0x39, 0x2f, 0x21, 0x14, 0x13, 0x06, 0x5d,
	0x69, 0x31, 0x30, 0x23, 0x22, 0x15, 0x07, 0x5e, 0x6a, 0x72, 0x32, 0x24, 0x16, 0x08, 0x09, 0x5f,
	0x6b, 0x33, 0x25, 0x17, 0x18, 0x0b, 0x0a, 0x60, 0x6c, 0x34, 0x35, 0x26, 0x27, 0x19, 0x0c, 0x61,
	0x6d, 0x73, 0x28, 0x74, 0x1a, 0x0d, 0x62, 0x6e, 0x3a, 0x36, 0x1c, 0x1b, 0x75, 0x2b, 0x63, 0x76,
	0x55, 0x56, 0x77, 0x78, 0x79, 0x7a, 0x0e, 0x7b, 0x7c, 0x4f, 0x7d, 0x4b, 0x47, 0x7e, 0x7f, 0x6f,
	0x52, 0x53, 0x50, 0x4c, 0x4d, 0x48, 0x01, 0x45, 0x57, 0x4e, 0x51, 0x4a, 0x37, 0x49, 0x46, 0x54,
	0x80, 0x81, 0x82, 0x41, 0x54, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
	0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
	0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
	0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
	0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
	0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
	0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
	0x00, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
};

unsigned char sc_convert_1(unsigned char in)
{
	static int shifted = 0;

	if (shifted)
	{
		shifted = 0;
		return convert_table[in] | 0x80;
	}

	if (in == 0xF0)
	{
		shifted = 1;
		return 0;
	} else {
		return convert_table[in];
	} 
}

void kbd_inject_scancode (unsigned char sc)
{
	outputf("Buffering %02x", sc);
	kbd_inj_buffer[kbd_inj_end] = sc;
	kbd_inj_end += 1;
	kbd_inj_end %= sizeof(kbd_inj_buffer);
}

void kbd_inject_keysym(uint32_t k, int downflag)
{
	const char * c;

	if ((k & 0xFFFFFF00) == 0)
	{ 
		c = scancodes2[k & 0xFF][downflag ? 0 : 1];
	} else if ((k & 0xFFFFFF00) == 0xFF00) {
		c = scancodes2high[k & 0xFF][downflag ? 0 : 1];
	} else {
		return;
	}

	if (!c) return;

	if (kbd_mode == 1) {
		while (*c) {
			char cconv = sc_convert_1(*c);
			if (cconv) kbd_inject_scancode(cconv);
			c++;
		}
	} else {
		while (*c) {
			kbd_inject_scancode(*c);
			c++;
		}
	}
}

unsigned char kbd_get_injected_scancode()
{
	unsigned char b;

	if (kbd_inj_end != kbd_inj_start)
	{
		b = kbd_inj_buffer[kbd_inj_start];
		kbd_inj_start += 1;
		kbd_inj_start %= sizeof(kbd_inj_buffer);
		outputf("Injecting %02x", b);
		return b;
	} else {
		outputf("Not injecting");
		return 0;
	}
}

int kbd_has_injected_scancode()
{
	if (kbd_inj_end != kbd_inj_start)
	{
		return 1;
	} else {
		return 0;
	}
}
