#pragma once

#include <string>
#include <memory>
#include <chrono>
#include <unordered_map>
#include <boost/optional.hpp>

#include "types.h"

namespace NVocal {

class TClientInfoStorage {
public:
    TClientInfoStorage(const std::string& storageDir);
    ~TClientInfoStorage();
    void Put(const TClientInfo& clientInfo);
    boost::optional<TClientInfo> Get(const std::string& login);
};

class TMessageStorage {
public:
    TMessageStorage(const std::string& storageDir);
    ~TMessageStorage();
    void Put(const std::string& login, const std::string& message);
    std::vector<std::string> GetMessages(const std::string& login,
                                         std::chrono::microseconds from,
                                         std::chrono::microseconds to);
};

class TSelfStorage {
public:
    TSelfStorage(const std::string& storageDir);
    ~TSelfStorage();
    void GenerateKeys();
    const std::string& GetPublickKey();
    const std::string& GetPrivateKey();
};

} // NVocal
