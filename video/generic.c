#include <stdint.h>
#include <fb.h>
#include <crc32.h>

uint32_t checksum_rect_generic32(int x, int y, int width, int height) {

	/* Generic checksum_rect function for video modes with 32-bit pixels
	 * (i.e. fb->curmode.bytestride = 4).
	 */

        int scanline = fb->curmode.xres * 4;
        unsigned char * lineaddr;
        int i;

        uint32_t sum = 0;

        for (i = 0; i < height; i++) {
                lineaddr = fb->fbaddr + (i + y) * scanline;

                sum ^= crc32(lineaddr, width * 4);
        }

        return sum;
}

void copy_pixels_generic32(char *buf, int x, int y, int width, int height)
{
	int cx, cy;
	unsigned int *ibuf = (unsigned int *)buf;
	unsigned int *fbuf;
	for (cy = y; cy < (y + height); cy++)
	{
		fbuf = (unsigned int *)fb->fbaddr;
		fbuf += cy * (fb->curmode.xres) + x;
		for (cx = x; cx < (x + width); cx++)
			*(ibuf++) = *(fbuf++);
	}
}
