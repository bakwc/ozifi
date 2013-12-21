#include <utils/exception.h>
#include <contrib/snappy/snappy.h>
#include <contrib/libspeex/speex.h>
#include <assert.h>
#include "compress.h"

namespace NVocal {

std::string Compress(const std::string& data) {
    std::string result;
    if (snappy::Compress(data.data(), data.size(), &result) <= 0) {
        throw UException("failed to compress data");
    }
    return result;
}

std::string Decompress(const std::string& data) {
    std::string result;
    if (!snappy::Uncompress(data.data(), data.size(), &result)) {
        throw UException("failed to decompress data");
    }
    return result;
}

TAudioCodec::TAudioCodec()
    : FrameSize(0)
    , EncoderState( speex_encoder_init(&speex_nb_mode))
    , DecoderState(speex_decoder_init(&speex_nb_mode))
    , BitsEnc(new SpeexBits())
    , BitsDec(new SpeexBits())
{
    int quality = 8;
    int ench = 1;
    speex_encoder_ctl(EncoderState, SPEEX_SET_QUALITY, &quality);
    speex_decoder_ctl(DecoderState, SPEEX_SET_ENH, &ench);
    speex_bits_init(BitsEnc.get());
    speex_bits_init(BitsDec.get());
    speex_encoder_ctl(EncoderState, SPEEX_GET_FRAME_SIZE, &FrameSize);
    assert(FrameSize != 0 && "invalid frame size");
}

TAudioCodec::~TAudioCodec() {
}

std::string TAudioCodec::CompressFrame(const std::string& data) {
    assert(data.size() == FrameSize && "Wrong data size");
    std::string result;
    const short* curData = reinterpret_cast<const short*>(data.data());
    result.resize(data.size());
    speex_bits_reset(BitsEnc.get());
    speex_encode_int(EncoderState, const_cast<short*>(curData), BitsEnc.get());   // todo: handle errors
    int nbBytes = speex_bits_write(BitsEnc.get(), &result[0], result.length());
    result.resize(nbBytes);
    return result;
}


std::string TAudioCodec::DecompressFrame(const std::string& data) {
    speex_bits_read_from(BitsDec.get(), const_cast<char*>(data.data()), data.size());
    std::string result;
    result.resize(FrameSize);
    const short* curData = reinterpret_cast<const short*>(data.data());
    speex_decode_int(DecoderState, BitsDec.get(), const_cast<short*>(curData));  // todo: handle errors
    return result;
}

}
