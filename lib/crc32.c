#include <stdint.h>

/* code from http://www.faqs.org/faqs/compression-faq/part1/section-26.html,
 * presumed public domain */

static uint32_t crc32_table[256];

uint32_t crc32(uint8_t *buf, int len, uint32_t crc0)
{
	uint8_t *p;
	uint32_t crc;

	crc = crc0;
	for (p = buf; len > 0; ++p, --len)
		crc = (crc << 8) ^ crc32_table[(crc >> 24) ^ *p];
	return crc;
}

/*
 * Build auxiliary table for parallel byte-at-a-time CRC-32.
 */
#define CRC32_POLY 0x04c11db7     /* AUTODIN II, Ethernet, & FDDI */

void crc32_init()
{
	int i, j;
	uint32_t c;

	for (i = 0; i < 256; ++i) {
		for (c = i << 24, j = 8; j > 0; --j)
			c = c & 0x80000000 ? (c << 1) ^ CRC32_POLY : (c << 1);
		crc32_table[i] = c;
	}
}
