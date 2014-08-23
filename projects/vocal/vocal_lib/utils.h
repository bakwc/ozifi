#pragma once

#include <string>
#include <projects/vocal/vocal_lib/vocal.pb.h>
#include <queue>
#include "defines.h"
#include <utils/buffer.h>
#include <iostream>

namespace NVocal {

class TAudioQueue {
      std::deque<char> Data;
public:
    void Add(TBuffer buff) {
        Data.insert(Data.end(), buff.Data(), buff.Data() + buff.Size());
    }

    void Get(char *array, size_t bytesToRead) {
        if (Data.size() < bytesToRead) {
            Data.insert(Data.end(), bytesToRead - Data.size(), 0);
        }
        std::copy(Data.begin(), Data.begin() + bytesToRead, array);
        Data.erase(Data.begin(), Data.begin()+bytesToRead);
    }

    bool Has(size_t bytesToRead) {
        return Data.size() >= bytesToRead;
    }

    size_t Size() {
        return Data.size();
    }
};

std::pair<std::string, std::string> GetLoginHost(const std::string& login);
bool GoodLogin(const std::string& login);

std::string RegisterResultToString(ERegisterResult res);

std::string LoginResultToString(ELoginResult res);

}
