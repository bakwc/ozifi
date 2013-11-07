#include <utils/exception.h>
#include <utils/cast.h>
#include <projects/vocal/vocal_lib/compress.h>
#include <projects/vocal/vocal_lib/crypto.h>
#include <projects/vocal/vocal_server_lib/storage.pb.h>
#include "storage.h"

using namespace std;

namespace NVocal {

// TClientInfoStorage

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
    if (!data.ParseFromString(Decompress(*strData))) {
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


// TMessageStorage

TMessageStorage::TMessageStorage(const string& storageDir)
    : Storage(NKwStorage::CreateLevelDbStorage(storageDir))
{
}

TMessageStorage::~TMessageStorage() {
}

void TMessageStorage::Put(const string& login,
                          const string& encryptedMessage,
                          chrono::microseconds date)
{
    TMessageData message;
    message.set_login(login);
    message.set_date(date.count());
    message.set_encryptedmessage(encryptedMessage);

    string key = login + ToString(date.count()) + ToString(LittleHash(encryptedMessage));
    Storage->Put(key, Compress(message.SerializeAsString()));
}

void TMessageStorage::PutFriendRequest(const string& login,
                                       const string& friendLogin,
                                       TDuration date)
{
    TMessageData message;
    message.set_login(login);
    message.set_date(date.MicroSeconds());
    message.set_friendrequestlogin(friendLogin);

    string key = login + ToString(date.MicroSeconds()) + ToString(LittleHash(friendLogin));
    Storage->Put(key, Compress(message.SerializeAsString()));
}

pair<TStringVector, TStringVector> TMessageStorage::GetMessages(const string& login,
                                                                TDuration from,
                                                                TDuration to)
{
    NKwStorage::TKwIterator it = Storage->Iterator();
    string fromStr = login + ToString(from.MicroSeconds());
    string toStr = login + ToString(to.MicroSeconds());
    vector<string> messages;
    vector<string> addFriendRequests;
    it->Seek(fromStr);
    while (true) {
        pair<string, string> value = it->Get();
        if (value.first > toStr) {
            break;
        }

        TMessageData data;
        if (!data.ParseFromString(Decompress(value.second))) {
            throw UException("failed to parse message");
        }
        if (data.has_encryptedmessage()) {
            messages.push_back(data.encryptedmessage());
        } else if (data.has_friendrequestlogin()) {
            addFriendRequests.push_back(data.friendrequestlogin());
        } else {
            assert(!"missing data");
        }
        it->Next();
        if (it->End()) {
            break;
        }
    }
    return pair<TStringVector, TStringVector>(messages, addFriendRequests);
}

// TSelfStorage

TSelfStorage::TSelfStorage(const std::string& storageDir)
    : Storage(NKwStorage::CreateLevelDbStorage(storageDir))
{
}

TSelfStorage::~TSelfStorage() {
}

void TSelfStorage::GenerateKeys() {
    std::pair<string, string> keys;
    keys = NVocal::GenerateKeys();
    Storage->Put("private_key", keys.first);
    Storage->Put("public_key", keys.second);
}

bool TSelfStorage::HasKeys() {
    boost::optional<string> pubKey = Storage->Get("public_key");
    boost::optional<string> privKey = Storage->Get("private_key");
    return (pubKey.is_initialized() && privKey.is_initialized());
}

string TSelfStorage::GetPublicKey() {
    boost::optional<string> pubKey = Storage->Get("public_key");
    if (!pubKey.is_initialized()) {
        throw UException("no keys found");
    }
    return *pubKey;
}

string TSelfStorage::GetPrivateKey() {
    boost::optional<string> privKey = Storage->Get("private_key");
    if (!privKey.is_initialized()) {
        throw UException("no keys found");
    }
    return *privKey;
}

} // NVocal
