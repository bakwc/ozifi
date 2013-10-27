#include <iostream>
#include <utils/string.h>
#include <library/captcha/captcha.h>
#include <library/image/image.h>

using namespace std;

int main(int argc, char **argv) {
    TCaptcha captcha = GenerateCaptcha();
    cout << captcha.Text << "\n";
    SaveFile("image.png", captcha.PngImage);
    return 0;
}
