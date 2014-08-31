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

template<typename T>
class TInfoStorage {
public:
    TInfoStorage(const std::string& storageDir);
    ~TInfoStorage();
    void Put(const T& clientInfo);
    bool Exists(const std::string& login);
    boost::optional<T> Get(const std::string& login);
private:
    std::unique_ptr<NKwStorage::TKwStorage> Storage;
};

using TClientInfoStorage = TInfoStorage<TClientInfo>;
using TConferenceInfoStorage = TInfoStorage<TConferenceInfo>;

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
