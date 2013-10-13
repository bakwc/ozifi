#pragma once

#include <string>
#include <functional>
#include <unordered_map>

namespace NVocal {

enum EClientState {
    CS_UnAuthorized,
    CS_Disconnected,
    CS_Connecting,
    CS_Logining,
    CS_Registering,
    CS_Connected
};

enum ELoginResult {
    LR_ConnectionFailure,
    LR_WrongCaptcha,
    LR_WrongLoginPassword,
    LR_Success
};

enum ERegisterResult {
    RR_ConnectionFailure,
    RR_WrongCaptcha,
    RR_LoginReserved,
    RR_Success
};

enum EFriendStatus {
    FS_Offline,
    FS_Busy,
    FS_Away,
    FS_Available
};

typedef std::function<void(const std::string&)> TDataCallback;
typedef std::function<void(ELoginResult)> TLoginCallback;
typedef std::function<void(ERegisterResult)> TRegisterCallback;
typedef std::function<std::string(size_t)> TDataRequireCallback;

struct TClientConfig {
    std::string SerializedState;                // internal state data, could be empty on first start
    TDataCallback StateChangeCallback;          // should be saved somewhere when changes
    TDataCallback CaptchaAvailableCallback;     // on captcha available (for login, reigster, etc.)
    TLoginCallback LoginResultCallback;         // on login failed / success
    TRegisterCallback RegisterResultCallback;   // on register success / fail
};

class TFriend {
public:
    const std::string& GetLogin();
    const std::string& GetName();
    EFriendStatus GetStatus();
    const std::string& GetStatusMessage();
    void SendMessage(const std::string& message);
    void SendFile(const std::string& name,
                  size_t size,
                  TDataRequireCallback fileDataCallback);
    void VoiceCall(TDataRequireCallback audioDataRequireCallback,
                   TDataCallback audioDataCallback);
    void VideoCall(TDataRequireCallback videoDataRequireCallback,
                   TDataRequireCallback audioDataRequireCallback,
                   TDataCallback audioDataCallback,
                   TDataCallback videoDataCallback);
};

typedef std::unordered_map<std::string, TFriend> TFriends;
typedef std::unordered_map<std::string, TFriend>::iterator TFriendIterator;

class TClient {
public:
    TClient(const TClientConfig& config);
    EClientState GetState();
    void Connect();                                     // initialize connection
    void Disconnect();                                  // initialize disconnection
    void Login(const std::string& login);               // initialize login
    void Login(const std::string& login,                // continue login
               const std::string& password,
               const std::string& captcha);
    void Register(const std::string& preferedLogin);    // initialize registration
    void Register(const std::string& preferedLogin,     // continue registration
                  const std::string& preferedPassword,
                  const std::string& email,
                  const std::string& captcha);
    void AddFriend(const std::string& friendLogin);     // initialize adding friend
    void AddFriend(const std::string& friendLogin,      // continue adding friend
                   const std::string& requestMessage,
                   const std::string& captcha);
    void RemoveFriend(const std::string& friendLogin);  // remove friend from friendlist
    TFriend& Friend(const std::string& login);          // friend by login
    TFriendIterator FriendsBegin();                     // use it to iterate throw friend
    TFriendIterator FriendsEnd();
private:
    TFriends Friends;
};

} // NVocal
