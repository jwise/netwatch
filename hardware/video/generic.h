#ifndef _CHECKSUM_RECT_C
#define _CHECKSUM_RECT_H

uint32_t checksum_rect_generic32(int x, int y, int width, int height);
void copy_pixels_generic32(char *buf, int x, int y, int width, int height);

#endif
