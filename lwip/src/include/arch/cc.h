#ifndef _ARCH_CC_H
#define _ARCH_CC_H

#include <stdint.h>
#include <vga-overlay.h>
#include <minilib.h>

typedef uint8_t u8_t;
typedef uint16_t u16_t;
typedef uint32_t u32_t;
typedef int8_t s8_t;
typedef int16_t s16_t;
typedef int32_t s32_t;
typedef uint32_t mem_ptr_t;

#define PACK_STRUCT_FIELD(x) x __attribute__((packed))
#define PACK_STRUCT_STRUCT __attribute__((packed))
#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_END

#define BYTE_ORDER LITTLE_ENDIAN

#ifndef NULL
#define NULL 0
#endif

#define LWIP_PLATFORM_BYTESWAP 1
#define LWIP_PLATFORM_HTONS(x) htons(x)
#define LWIP_PLATFORM_HTONL(x) htonl(x)

#define LWIP_PLATFORM_DIAG(x) dolog(x)
#define LWIP_PLATFORM_ASSERT(x) dologf("ASSERT FAILED: %s\n", (x));

#endif
