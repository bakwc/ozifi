#pragma once

#include <memory>
#include <utils/buffer.h>

class TImageImpl;
class TImage {
public:
    TImage();
    ~TImage();
    void LoadPng(const TBuffer& data);
    void LoadPng(const std::string& fileName);
    void LoadJpg(const TBuffer& data);
    void LoadJpg(const std::string& fileName);
    std::string SavePng();
    void SavePng(const std::string& fileName);
private:
    std::unique_ptr<TImageImpl> Impl;
};
