#pragma once

#include <unordered_map>
#include <string>
#include <vector>

#include "message.h"
#include "callback.h"

namespace NVocal {

enum EFriendStatus {
    FS_Offline,
    FS_Unauthorized,
    FS_Busy,
    FS_Away,
    FS_Available
};

class TFriend {
public:
    TFriend(const std::string& login, EFriendStatus status);
    const std::string& GetLogin();
    const std::string& GetName();
    EFriendStatus GetStatus();
    const std::string& GetStatusMessage();
    std::vector<TMessage> GetHistory(); // old messages
    void SendMssg(const std::string& message);
    void SendFile(const std::string& name,
                  size_t size,
                  TDataRequireCallback fileDataCallback);
    void StartCall(TDataRequireCallback videoDataRequireCallback, // same function for accept call
                   TDataRequireCallback audioDataRequireCallback,
                   TDataCallback audioDataCallback,
                   TDataCallback videoDataCallback,
                   TBoolCallback partnerVideoStatusCallback,
                   bool videoEnabled);
    void EnableVideo();
    void DisableVideo();
    void FinishCall();
private:
    std::string Login;
    std::string Name;
    EFriendStatus Status;
};

typedef std::unordered_map<std::string, TFriend> TFriends;
typedef TFriends::iterator TFriendIterator;

} // NVocal
