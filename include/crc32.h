#ifndef _CRC32_H
#define _CRC32_H

#include <stdint.h>

extern uint32_t crc32(unsigned char *buf, int len);
extern void crc32_init();

#endif

