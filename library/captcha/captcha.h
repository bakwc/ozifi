#pragma once

#include <string>
#include <ctime>

struct TCaptcha {
    std::string Text;
    std::string PngImage;
};

TCaptcha GenerateCaptcha(); // returns captcha in png format, 200x70
