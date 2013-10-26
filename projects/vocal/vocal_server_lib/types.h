#pragma once

#include <string>
#include <unordered_map>

#include <projects/vocal/vocal_server_lib/storage.pb.h>

namespace NVocal {

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
