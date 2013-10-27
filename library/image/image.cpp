#include <contrib/lodepng/lodepng.h>
#define cimg_display 0
#include <contrib/cimg/cimg.h>
#include <contrib/jpeg/jpgd.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include <contrib/stb_truetype/stb_truetype.h>
#include <utils/exception.h>
#include <utils/string.h>

#include "image.h"

using namespace std;

typedef cimg_library::CImg<unsigned char> TCImg;
class TImageImpl {
public:
    TImageImpl() {
    }
    TImageImpl(size_t width, size_t height) {
        Image = TCImg(width, height, 1, 4);
    }
    void LoadPng(const TBuffer& data) {
        vector<unsigned char> image;
        unsigned int width, height;
        size_t err = lodepng::decode(image, width, height, (const unsigned char*)data.Data(), data.Size(), LCT_RGBA, 8);
        if (err) {
            throw UException(lodepng_error_text(err));
        }
        Image = TCImg(width, height, 1, 4);
        unsigned char* r = Image.data(0, 0, 0, 0);
        unsigned char* g = Image.data(0, 0, 0, 1);
        unsigned char* b = Image.data(0, 0, 0, 2);
        unsigned char* a = Image.data(0, 0, 0, 3);
        const unsigned char* s;
        for (s = image.data(); s < image.data() + image.size();) {
            *(r++) = *(s++);
            *(g++) = *(s++);
            *(b++) = *(s++);
            *(a++) = *(s++);
        }
    }

    void LoadPng(const std::string& fileName) {
        string data = LoadFile(fileName);
        LoadPng(TBuffer(data.data(), data.size()));
    }

    void LoadJpg(const TBuffer& data) {
        int width, height, comps;
        unsigned char* image;
        image = jpgd::decompress_jpeg_image_from_memory((const unsigned char*)data.Data(),
                                                        data.Size(),
                                                        &width,
                                                        &height,
                                                        &comps,
                                                        4);
        if (!image) {
            throw UException("Failed to load jpeg");
        }
        Image = TCImg(width, height, 1, 4);
        unsigned char* r = Image.data(0, 0, 0, 0);
        unsigned char* g = Image.data(0, 0, 0, 1);
        unsigned char* b = Image.data(0, 0, 0, 2);
        unsigned char* a = Image.data(0, 0, 0, 3);
        const unsigned char* s;
        for (s = image; s < image + width * height * 4;) {
            *(r++) = *(s++);
            *(g++) = *(s++);
            *(b++) = *(s++);
            *(a++) = *(s++);
        }
    }

    void LoadJpg(const std::string& fileName) {
        string data = LoadFile(fileName);
        LoadJpg(TBuffer(data.data(), data.size()));
    }

    std::string SavePng() {
        vector<unsigned char> result(Image.width() * Image.height() * 4);
        unsigned char* d;
        const unsigned char* r = Image.data(0, 0, 0, 0);
        const unsigned char* g = Image.data(0, 0, 0, 1);
        const unsigned char* b = Image.data(0, 0, 0, 2);
        int sp = Image.spectrum();
        if (sp == 4) {
            const unsigned char* a = Image.data(0, 0, 0, 3);
            for (d = result.data(); d < result.data() + result.size();) {
                *(d++) = *(r++);
                *(d++) = *(g++);
                *(d++) = *(b++);
                *(d++) = *(a++);
            }
        } else if (sp == 3) {
            for (d = result.data(); d < result.data() + result.size();) {
                *(d++) = *(r++);
                *(d++) = *(g++);
                *(d++) = *(b++);
                *(d++) = 255;
            }
        } else {
            throw UException("failed to save image - wrong type");
        }
        vector<unsigned char> out;
        size_t err = lodepng::encode(out, result, Image.width(), Image.height());
        if (err) {
            throw UException(lodepng_error_text(err));
        }
        string data(out.begin(), out.end());
        return data;
    }

    void SavePng(const std::string& fileName) {
        string data = SavePng();
        SaveFile(fileName, data);
    }

    void SetPixel(size_t x, size_t y, ui16 r, ui16 g, ui16 b) {
        unsigned char* rp = Image.data(0, 0, 0, 0);
        unsigned char* gp = Image.data(0, 0, 0, 1);
        unsigned char* bp = Image.data(0, 0, 0, 2);
        unsigned char* ap = Image.data(0, 0, 0, 3);
        size_t ofset = y * Image.width() + x;
        rp += ofset;
        gp += ofset;
        bp += ofset;
        ap += ofset;
        *rp = r;
        *gp = g;
        *bp = b;
        *ap = 255;
    }

    void Fill(ui16 r, ui16 g, ui16 b) {
        unsigned char* rp = Image.data(0, 0, 0, 0);
        unsigned char* gp = Image.data(0, 0, 0, 1);
        unsigned char* bp = Image.data(0, 0, 0, 2);
        unsigned char* ap = Image.data(0, 0, 0, 3);
        memset(rp, r, Image.width() * Image.height());
        memset(gp, g, Image.width() * Image.height());
        memset(bp, b, Image.width() * Image.height());
        memset(ap, 255, Image.width() * Image.height());
    }

private:
     TCImg Image;
};

TImage::TImage() {
    Impl.reset(new TImageImpl());
}

TImage::TImage(size_t width, size_t height) {
    Impl.reset(new TImageImpl(width, height));
}

TImage::~TImage() {
}

void TImage::LoadPng(const TBuffer& data) {
    Impl->LoadPng(data);
}

void TImage::LoadPng(const std::string& fileName) {
    Impl->LoadPng(fileName);
}

void TImage::LoadJpg(const TBuffer& data) {
    Impl->LoadJpg(data);
}

void TImage::LoadJpg(const std::string& fileName) {
    Impl->LoadJpg(fileName);
}

std::string TImage::SavePng() {
    return Impl->SavePng();
}

void TImage::SavePng(const std::string& fileName) {
    Impl->SavePng(fileName);
}

void TImage::SetPixel(size_t x, size_t y, ui16 r, ui16 g, ui16 b) {
    Impl->SetPixel(x, y, r, g, b);
}

void TImage::Fill(ui16 r, ui16 g, ui16 b) {
    Impl->Fill(r, g, b);
}
