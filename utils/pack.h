#pragma once

#include <string>
#include <cstring>

#include "types.h"

// todo: make platform-independent

template<typename T>
std::string PackTransformed(T val) {
    std::string result;
    size_t length = sizeof(val);
    result.resize(length);
    memcpy(&result[0], &val, length);
    return result;
}

inline void SwapByteOrder(ui8&) {
}

inline void SwapByteOrder(ui16& us)
{
    us = (us >> 8) |
         (us << 8);
}

inline void SwapByteOrder(ui32& ui)
{
    ui = (ui >> 24) |
         ((ui<<8) & 0x00FF0000) |
         ((ui>>8) & 0x0000FF00) |
         (ui << 24);
}

inline void SwapByteOrder(ui64& ull)
{
    ull = (ull >> 56) |
          ((ull<<40) & 0x00FF000000000000) |
          ((ull<<24) & 0x0000FF0000000000) |
          ((ull<<8) & 0x000000FF00000000) |
          ((ull>>8) & 0x00000000FF000000) |
          ((ull>>24) & 0x0000000000FF0000) |
          ((ull>>40) & 0x000000000000FF00) |
          (ull << 56);
}

template<typename T>
std::string Pack(T val) {
    SwapByteOrder(val);
    return PackTransformed(val);
}
