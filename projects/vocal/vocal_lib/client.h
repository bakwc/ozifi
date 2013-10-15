#pragma once

#include <string>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <chrono>
#include <boost/optional.hpp>
#include <utils/buffer.h>

namespace NVocal {

// General

struct TMessage {
    std::string Message;
    std::string From;                       // sender login
    std::string To;                         // destination login or chat id
    std::chrono::microseconds Timestamp;    // sender timestamp (consider is real; auto-synced with server)
    boost::array<char, 16> UID;             // random id
};

typedef std::function<void(const TBuffer& /*data*/)> TDataCallback;
typedef TDataCallback TNamedCallback;
typedef std::function<void(const std::string& /*name*/, const TBuffer& /*data*/)> TNamedDataCallback;
typedef std::function<std::string(size_t)> TDataRequireCallback;
typedef std::functiom<void(bool)> TBoolCallback;
typedef std::functiom<void(const std::string& /*name*/, bool /*result*/)> TNamedBoolCallback;
typedef std::function<void(const std::string&, const TMessage&)> TMessageCallback;


// Friend

enum EFriendStatus {
    FS_Offline,
    FS_Unauthorized,
    FS_Busy,
    FS_Away,
    FS_Available
};

class TFriend {
public:
    const std::string& GetLogin();
    const std::string& GetName();
    EFriendStatus GetStatus();
    const std::string& GetStatusMessage();
    std::vector<TMessage> GetHistory(); // old messages
    void SendMessage(const std::string& message);
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
};

typedef std::unordered_map<std::string, TFriend> TFriends;
typedef TFriends::iterator TFriendIterator;


// Conference

typedef std::unordered_set<std::string> TStringSet;
typedef TStringSet TStringSetIterator;
typedef TStringSetIterator TParticipantIterator;

class TConference {
public:
    std::string GetId();
    std::string GetAddress();
    const std::string& GetTopic();
    void SetTopic(const std::string& topic);
    void AddFriend(const std::string& friendLogin);
    TParticipantIterator ParticipantsBegin();
    TParticipantIterator ParticipantsEnd();
    void StartCall(TDataRequireCallback videoDataRequireCallback, // same function for accept call
                   TDataRequireCallback audioDataRequireCallback,
                   TDataCallback audioDataCallback,
                   TNamedDataCallback videoDataCallback,
                   TNamedBoolCallback partnerVideoStatusCallback,
                   bool videoEnabled);
    void FinishCall();
    std::vector<TMessage> GetHistory(); // old messages
    void SendMessage(const TMessage& message);
private:
    std::string Id;
    boost::optional<std::string> Name;
    std::string Topic;
    TStringSet Participants;
    std::string Server;
};

typedef std::unordered_map<std::string, TConference> TConferences;
typedef TConferences::iterator TConferenceIterator;


// Client

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

typedef std::function<void(ELoginResult)> TLoginCallback;
typedef std::function<void(ERegisterResult)> TRegisterCallback;

struct TClientConfig {
    std::string SerializedState;                // internal state data, could be empty on first start
    TDataCallback StateChangeCallback;          // should be saved somewhere when changes
    TDataCallback CaptchaAvailableCallback;     // on captcha available (for login, reigster, etc.)
    TLoginCallback LoginResultCallback;         // on login failed / success
    TRegisterCallback RegisterResultCallback;   // on register success / fail
    TNamedCallback CallCallback;                // on incoming call
    TNamedCallback ConferenceCallCallback;      // on incoming conference call
    TNamedCallback ConferenceJoinCallback;      // on join to conference
    TNamedCallback ConferenceLeftCallback;      // on conference left
    TMessageCallback MessageCallback;           // on message received
    TMessageCallback ConferenceMessageCallback; // on conference message received
    TMessageCallback FriendRequestCallback;     // on friend request received
};

class TClient {
public:
    TClient(const TClientConfig& config);
    EClientState GetState();

    // connection
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

    // friends
    void AddFriend(const std::string& friendLogin);     // initialize adding friend
    void AddFriend(const std::string& friendLogin,      // continue adding friend
                   const std::string& requestMessage,
                   const std::string& captcha);
    void RemoveFriend(const std::string& friendLogin);  // remove friend from friendlist
    TFriend& GetFriend(const std::string& login);       // friend by login
    TFriendIterator FriendsBegin();                     // use it to iterate over friends
    TFriendIterator FriendsEnd();

    // conference
    void CreateConference();                            // init new conference creation
    void LeaveConference(const std::string& id);        // leaves conference
    TConference& GetConference(const std::string& id);  // conference by id
    TConferenceIterator ConferencesBegin();             // use it to iterate over conferences
    TConferenceIterator ConferencesEnd();
private:
    TFriends Friends;
    TConferences Conferences;
};

} // NVocal
