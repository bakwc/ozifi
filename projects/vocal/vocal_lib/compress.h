#pragma once

#include <memory>
#include <string>

class SpeexBits;

namespace NVocal {

std::string Compress(const std::string& data);
std::string Decompress(const std::string& data);

class TAudioCodec {
public:
    TAudioCodec();
    ~TAudioCodec();
    std::string CompressFrame(const std::string& data);
    std::string DecompressFrame(const std::string& data);
    inline size_t GetFrameSize() {
        return FrameSize;
    }
private:
    size_t FrameSize;
    void* EncoderState;
    void* DecoderState;
    std::unique_ptr<SpeexBits> BitsEnc;
    std::unique_ptr<SpeexBits> BitsDec;
};
}
