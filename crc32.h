#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

uint32_t
crc32(uint32_t crc, const void *buf, size_t size);

#ifdef __cplusplus
}
#endif
