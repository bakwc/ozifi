#pragma once

#include <memory>
#include <string>
#include <utils/buffer.h>

namespace NVocal {

std::string Compress(const std::string& data);
std::string Decompress(const std::string& data);

class TCodec {
public:
    virtual void Encode(TBuffer data, std::string& out) = 0;
    virtual void Decode(TBuffer data, std::string& out) = 0;
};

TCodec* CreateOpusCodec();

}
