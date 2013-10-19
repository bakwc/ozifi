#pragma once

#include <string>
#include <memory>
#include <chrono>
#include <unordered_map>
#include <boost/optional.hpp>

namespace NVocal {

enum EFriendType {
    FT_Friend,
    FT_Conference
};

struct TFriendInfo {
    std::string Login;
    std::string EncryptedKey;
    EFriendType Type;
};

struct TClientInfo {
    std::string Login;
    std::unordered_map<std::string, TFriendInfo> Friends; // and conferences
    std::string PublicKey;
    std::string EncryptedPrivateKey;
    std::string LoginPasswordHash;
};

class TClientInfoStorageImpl;
class TClientInfoStorage {
public:
    TClientInfoStorage(const std::string& storageDir);
    ~TClientInfoStorage();
    void Put(const TClientInfo& clientInfo);
    boost::optional<TClientInfo> Get(const std::string& login);
private:
    std::unique_ptr<TClientInfoStorageImpl> Impl;
};

class TMessageStorageImpl;
class TMessageStorage {
public:
    TMessageStorage(const std::string& storageDir);
    ~TMessageStorage();
    void Put(const std::string& login, const std::string& message);
    std::vector<std::string> GetMessages(const std::string& login,
                                         std::chrono::microseconds from,
                                         std::chrono::microseconds to);
private:
    std::unique_ptr<TMessageStorageImpl> Impl;
};

class TServerStatusStorageImpl;
class TServerStatusStorage {
public:
    TServerStatusStorage(const std::string& storageDir);
    ~TServerStatusStorage();
    void GenerateKeys();
    void SetHostname(const std::string& hostname);
    const std::string& GetHostname();
    const std::string& GetPublickKey();
    const std::string& GetPrivateKey();
private:
    std::unique_ptr<TServerStatusStorageImpl> Impl;
};

} // NVocal
