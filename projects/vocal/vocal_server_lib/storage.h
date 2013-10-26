#pragma once

#include <string>
#include <memory>
#include <chrono>
#include <unordered_map>
#include <boost/optional.hpp>
#include <library/kwstorage/leveldb.h>

#include "types.h"

namespace NVocal {

class TClientInfoStorage {
public:
    TClientInfoStorage(const std::string& storageDir);
    ~TClientInfoStorage();
    void Put(const TClientInfo& clientInfo);
    bool Exists(const std::string& login);
    boost::optional<TClientInfo> Get(const std::string& login);
private:
    std::unique_ptr<NKwStorage::TKwStorage> Storage;
};

class TMessageStorage {
public:
    TMessageStorage(const std::string& storageDir);
    ~TMessageStorage();
    void Put(const std::string& login, const std::string& message);
    std::vector<std::string> GetMessages(const std::string& login,
                                         std::chrono::microseconds from,
                                         std::chrono::microseconds to);
private:
    std::unique_ptr<NKwStorage::TKwStorage> Storage;
};

class TSelfStorage {
public:
    TSelfStorage(const std::string& storageDir);
    ~TSelfStorage();
    void GenerateKeys();
    std::string GetPublicKey();
    std::string GetPrivateKey();
private:
    std::unique_ptr<NKwStorage::TKwStorage> Storage;
};

} // NVocal
