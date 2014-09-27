#pragma once
#ifdef __ANDROID__

#define int8_t char
#define int16_t short
#define int32_t int
#define int64_t long long

#define uint8_t unsigned char
#define uint16_t unsigned short
#define uint32_t unsigned int
#define uint64_t unsigned long long

#else
#include <cinttypes>
#endif