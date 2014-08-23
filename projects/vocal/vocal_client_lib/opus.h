#pragma once
#include <utils/buffer.h>
#include <string>

class OpusEncoder;
class OpusDecoder;
class TOpus {
public:
    TOpus();
    void Encode(TBuffer data, std::string& out);
    void Decode(TBuffer data, std::string& out);
private:
    OpusEncoder* Encoder;
    OpusDecoder* Decoder;
};

