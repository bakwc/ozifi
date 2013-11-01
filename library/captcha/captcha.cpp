#include <library/image/image.h>

// todo: implement captcha using image library functions

#include "captcha.h"

using namespace std;

extern "C" void captcha(unsigned char im[70*200], unsigned char l[6]);

struct TRandomizer {
    inline TRandomizer() {
        srand(time(NULL));
    }
} GRandomizer;

TCaptcha GenerateCaptcha() {
    TImage image(200, 70);
    char l[6];
    unsigned char im[70*200];
    captcha(im, (unsigned char*)l);
    TCaptcha result;
    result.Text = string(l, 5);
    for (size_t i = 0; i < 200; ++i) {
        for (size_t j = 0; j < 70; ++j) {
            unsigned char c = im[j * 200 + i];
            image.SetPixel(i, j, c, c, c);
        }
    }
    result.PngImage = image.SavePng();
    return result;
}
