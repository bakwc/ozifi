#pragma once

#include <memory>
#include <utils/buffer.h>

class TImageImpl;
class TImage {
public:
    TImage();
    TImage(size_t width, size_t height);
    ~TImage();
    void LoadPng(const TBuffer& data);
    void LoadPng(const std::string& fileName);
    void LoadJpg(const TBuffer& data);
    void LoadJpg(const std::string& fileName);
    std::string SavePng();
    void SavePng(const std::string& fileName);
    void SetPixel(size_t x, size_t y, ui16 r, ui16 g, ui16 b);
    void Fill(ui16 r, ui16 g, ui16 b);
private:
    std::unique_ptr<TImageImpl> Impl;
};
