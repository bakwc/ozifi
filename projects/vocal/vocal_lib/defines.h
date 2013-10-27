#pragma once

#include <string>
#include <utils/types.h>

namespace NVocal {

enum ERequestType {
    RT_Register,
    RT_Login,
    RT_Authorize
};

enum ERegisterResult {
    RR_ConnectionFailure,
    RR_Success,
    RR_WrongCaptcha,
    RR_WrongLogin
};

const size_t DEFAULT_RANDOM_SEQUNCE_BITS = 4096;
const size_t DEFAULT_ASSYMETRICAL_KEY_LENGTH = 4096;
const std::string CRYPTO_SALT_1 = "NVocal";
const std::string CRYPTO_SALT_2 = "distributed messenger";
const std::string CRYPTO_SALT_3 = "!!WIN";

} // NVocal
