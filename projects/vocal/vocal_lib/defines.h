#pragma once

#include <utils/types.h>

namespace NVocal {

enum ERequestType {
    RT_Register,
    RT_Login,
    RT_Authorize
};

enum ERegisterResult {
    RR_Success,
    RR_WrongCaptcha,
    RR_WrongLogin
};

const size_t CAPTCHA_WIDTH = 90;
const size_t CAPTCHA_HEIGHT = 40;

} // NVocal
