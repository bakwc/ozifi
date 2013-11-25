#pragma once

#include <string>
#include <memory>
#include <chrono>
#include <unordered_map>
#include <boost/optional.hpp>
#include <utils/date_time.h>
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
    void Put(const std::string& login,
             const std::string& encryptedMessage,
             TDuration date);
    TStringVector GetMessages(const std::string& login,
                              TDuration from,
                              TDuration to);
private:
    std::unique_ptr<NKwStorage::TKwStorage> Storage;
};

class TSelfStorage {
public:
    TSelfStorage(const std::string& storageDir);
    ~TSelfStorage();
    void GenerateKeys();
    bool HasKeys();
    std::string GetPublicKey();
    std::string GetPrivateKey();
private:
    std::unique_ptr<NKwStorage::TKwStorage> Storage;
};

} // NVocal
