#pragma once

#include <string>

struct TCaptcha {
    std::string Text;
    std::string PngImage;
};

TCaptcha GenerateCaptcha(size_t width, size_t height); // returns captcha in png format
