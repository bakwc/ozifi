#pragma once

#include <string>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <chrono>
#include <mutex>
#include <boost/optional.hpp>
#include <utils/buffer.h>
#include <library/udt/client.h>
#include <projects/vocal/vocal_client_lib/state.pb.h>

#include "callback.h"
#include "message.h"
#include "config.h"
#include "friend.h"
#include "conference.h"
#include "state.h"

/** This is a main interface that should be used
 * to communicate with core in client applications. */

namespace NVocal {

class TNatPmp;
class TClient {
    friend class TFriend;
public:
    TClient(const TClientConfig& config);
    ~TClient();
    EClientState GetState();

    // connection
    void Connect();                                     // Connect to server, using stored login info.
                                                        // You can perform connection only after
                                                        // authorization / registration. Use
                                                        // HasConnectData() to check if connection
                                                        // can be performed.

    bool HasConnectData();                              // Check if has keys, host address and other
                                                        // data, required for connection.

    void Disconnect();                                  // Disconnect from server and friends

    void Login(const std::string& login);               // Start login procedure. It will send
                                                        // captcha if succes or calls
                                                        // LoginResultCallback with failure result.

    void Login(const std::string& password,             // Continue login. It will calls
               const std::string& captcha);             // LoginResultCallback on success or fail.

    void Register(const std::string& preferedLogin);    // Start registration procedure. It will send
                                                        // captcha if succes or calls
                                                        // RegisterResultCallback with failure result.

    void Register(const std::string& preferedPassword,  // Continue registration. It will calls
                  const std::string& email,             // RegisterResultCallback on success or fail.
                  const std::string& captcha);

    // friends
    size_t FriendsSize();                               // total number of friends
    void AddFriend(const std::string& friendLogin);     // Send friend add request or accept friend.
    void RemoveFriend(const std::string& friendLogin);  // Remove friend from the friendlist.
    TFriendRef GetFriend(const std::string& login);     // Returns friend by his login.
    TFriendIterator FriendsBegin();                     // Use it to iterate over friendlist.
    TFriendIterator FriendsEnd();

    // conference
    size_t ConferencesSize();                           // total number of conferences
    void CreateConference();                            // Init new conference creation.
    void LeaveConference(const std::string& id);        // Leaves conference.
    TConference& GetConference(const std::string& id);  // Returns conference by id.
    TConferenceIterator ConferencesBegin();             // Use it to iterate over conferences.
    TConferenceIterator ConferencesEnd();

    // other stuff
    std::string GetFullLogin();                         // Returns full client login, eg. login@exaple.com


protected:
    std::string GetLogin();
    std::string GetPublicKey();
    std::string GetPrivateKey();
    std::string GetHost();
    bool HasNatPmp();
    TNatPmp& GetNatPmp();
    TDuration GetTime();
private:
    void OnConnected(bool success);
    void OnDataReceived(const TBuffer& data);
    void OnPacketReceived(std::string&& packet);
    void OnDisconnected();
    void ForceDisconnect();
    void LoadState();
    void SaveState();
    void ConnectWithFriends();
    void OnFriendStatusChanged(TFriendRef frnd);
    void OnCallStatusChanged(TFriendRef frnd);
    void OnMessageReceived(const TMessage& message);
    void SendOfflineMessage(const std::string& friendLogin, const TBuffer& data);
private:
    EClientState CurrentState;
    std::string StateDir;
    TClientConfig Config;
    TFriends Friends;
    TConferences Conferences;
    std::unique_ptr<NUdt::TClient> UdtClient;
    std::unique_ptr<TNatPmp> NatPmp;
    TClientState State;
    std::string Buffer;
    std::mutex Lock;
    std::string Password;
};

} // NVocal
