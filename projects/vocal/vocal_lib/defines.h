#pragma once

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

const size_t CAPTCHA_WIDTH = 90;
const size_t CAPTCHA_HEIGHT = 40;
const size_t DEFAULT_RANDOM_SEQUNCE_BITS = 4096;
const size_t DEFAULT_ASSYMETRICAL_KEY_LENGTH = 4096;

} // NVocal
