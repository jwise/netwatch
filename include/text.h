#ifndef _TEXT_H

#include <stdint.h>

extern void text_init();
extern void text_render(unsigned char *buf, unsigned int x, unsigned int y, unsigned int w, unsigned int h);
extern uint32_t text_checksum(int x, int y, int w, int h);

#endif
