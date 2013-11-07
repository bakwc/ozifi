#pragma once

#include <string>
#include <unordered_map>

#include <projects/vocal/vocal_server_lib/storage.pb.h>

namespace NVocal {

struct TFriendInfo {
    std::string Login;
    std::string PublicKey;
    std::string EncryptedKey;
    EFriendType Type;
    bool Authorized;
};

typedef std::unordered_map<std::string, TFriendInfo> TFriendList;
typedef std::vector<std::string> TStringVector;

struct TClientInfo {
    std::string Login;
    TFriendList Friends; // and conferences
    std::string PublicKey;
    std::string EncryptedPrivateKey;
    std::string LoginPasswordHash;
};

} // NVocal
