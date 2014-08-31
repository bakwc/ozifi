#pragma once

#include <string>
#include <unordered_map>

#include <projects/vocal/vocal_lib/defines.h>
#include <projects/vocal/vocal_server_lib/storage.pb.h>

namespace NVocal {

struct TFriendInfo {
    inline TFriendInfo()
        : NeedOfflineKey(false)
    {
    }
    std::string Login;
    std::string PublicKey;
    std::string ServerPublicKey;
    // keys for offline messages
    std::string EncryptedKey;    // ourself keys, encrypted symmetrically with our password
    std::string OfflineKey;      // friends key, encrypted assymetrically with our private key
    std::string OfflineKeySignature; // friends offline key signature
    EFriendType Type;
    EAuthStatus AuthStatus;
    bool NeedOfflineKey;
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

struct TConferenceInfo {
    std::string ConferenceName;
    std::vector<std::string> Users;
    std::string Subject;
    std::string PasswordHash;
};

} // NVocal
