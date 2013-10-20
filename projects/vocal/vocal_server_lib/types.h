#pragma once

#include <string>
#include <unordered_map>

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

} // NVocal
