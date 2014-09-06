#include <utils/exception.h>
#include <contrib/snappy/snappy.h>
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



}
