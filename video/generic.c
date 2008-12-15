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
                lineaddr = fb->fbaddr + (i + y) * scanline + (4 * x);

                sum = crc32(lineaddr, width * 4, sum);
        }

        return sum;
}

void copy_pixels_generic32(char *buf, int x, int y, int width, int height)
{
	int cx, cy;
	unsigned int *ibuf = (unsigned int *)buf;
	unsigned int *fbuf;
	for (cy = 0; cy < height; cy++)
	{
		fbuf = (unsigned int *)fb->fbaddr;
		fbuf += (cy + y) * (fb->curmode.xres) + x;
		for (cx = 0; cx < width; cx++)
			*(ibuf++) = *(fbuf++);
	}
}
