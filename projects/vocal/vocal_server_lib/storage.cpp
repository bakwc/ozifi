#include <utils/exception.h>
#include <projects/vocal/vocal_lib/compress.h>
#include <projects/vocal/vocal_server_lib/storage.pb.h>
#include "storage.h"

using namespace std;

namespace NVocal {

TClientInfoStorage::TClientInfoStorage(const std::string& storageDir)
    : Storage(NKwStorage::CreateLevelDbStorage(storageDir))
{
}

TClientInfoStorage::~TClientInfoStorage() {
}

void TClientInfoStorage::Put(const TClientInfo& clientInfo) {
    TClientInfoData data;
    data.set_login(clientInfo.Login);
    data.set_publickkey(clientInfo.PublicKey);
    data.set_encryptedprivatekey(clientInfo.EncryptedPrivateKey);
    data.set_loginpasswordhash(clientInfo.LoginPasswordHash);
    for (auto& frnd: clientInfo.Friends) {
        TFriendInfoData* friendData = data.add_friends();
        friendData->set_login(frnd.second.Login);
        friendData->set_encryptedkey(frnd.second.EncryptedKey);
        friendData->set_type(frnd.second.Type);
    }
    Storage->Put(clientInfo.Login, Compress(data.SerializeAsString()));
}

bool TClientInfoStorage::Exists(const std::string& login) {
    return Storage->Exists(login);
}

boost::optional<TClientInfo> TClientInfoStorage::Get(const std::string& login) {
    boost::optional<string> strData = Storage->Get(login);
    if (!strData.is_initialized()) {
        return boost::optional<TClientInfo>();
    }
    TClientInfoData data;
    if (!data.ParseFromString(*strData)) {
        throw UException("failed to parse storage data");
    }
    TClientInfo result;
    result.Login = data.login();
    result.EncryptedPrivateKey = data.encryptedprivatekey();
    result.LoginPasswordHash = data.loginpasswordhash();
    result.PublicKey = data.publickkey();
    for (size_t i = 0; i < data.friends_size(); ++i) {
        const TFriendInfoData& frnd = data.friends(i);
        TFriendInfo& friendInfo = result.Friends[frnd.login()];
        friendInfo.Login = frnd.login();
        friendInfo.EncryptedKey = frnd.encryptedkey();
        friendInfo.Type = frnd.type();
    }
    return result;
}

} // NVocal
