#include <stdint.h>
#include <fb.h>

uint32_t checksum_rect_generic32(int x, int y, int width, int height) {

	/* Generic checksum_rect function for video modes with 32-bit pixels
	 * (i.e. fb->curmode.bytestride = 4).
	 */

        int scanline = fb->curmode.xres * 4;
        uint32_t * lineaddr;
        int i, j;

        uint32_t sum = 0;

        for (i = 0; i < height; i++) {
                lineaddr = (uint32_t *)(fb->fbaddr + (i + y) * scanline);

                for (j = 0; j < width; j++) {
                        sum += lineaddr[j + x];
                }
        }

        return sum;
}

