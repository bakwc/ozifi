#pragma once

#include "types.h"

#include <string>
#include <boost/array.hpp>

class TBuffer {
public:
    TBuffer(const char* data, size_t size)
        : DataPtr(data)
        , Length(size)
    {
    }
    TBuffer(const std::string& str)
        : DataPtr(str.data())
        , Length(str.size())
    {
    }
    inline char operator[](size_t pos) const {
        return DataPtr[pos];
    }
    inline const char* Data() const {
        return DataPtr;
    }
    inline size_t Size() const {
        return Length;
    }
private:
    const char* DataPtr;
    size_t Length;
};
