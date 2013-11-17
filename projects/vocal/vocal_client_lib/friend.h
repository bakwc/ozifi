#pragma once

#include <unordered_map>
#include <string>
#include <vector>

#include <boost/optional.hpp>
#include <library/udt/client.h>

#include "message.h"
#include "callback.h"

namespace NVocal {

enum EFriendStatus {
    FS_Offline,
    FS_AddRequest,
    FS_Unauthorized,
    FS_Busy,
    FS_Away,
    FS_Available
};

enum EConnectionStatus {
    COS_Offline,
    COS_ConnectingToServer,
    COS_WaitingFriendAddress,
    COS_ConnectingToFriend,
    COS_Connected
};

class TClient;
class TFriend {
    friend class TClient;
public:
    TFriend();
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
protected:
    /** connect to friend server and ask him to help to establish connection
     * and then connect to friend */
    void Connect();
    /** connect to local server when required and then connect to friend */
    void ConnectAccept();
    void OnConnected(bool success);
    void OnDataReceived(const TBuffer& data);
    void OnDisconnected();
    void ForceDisconnect();
    /** connect to friend and bind known local port to traverse nat */
    void ConnectThrowNat(const TNetworkAddress& address, ui16 localPort = 0);
private:
    void InitUdtClient();
protected:
    bool ToDelete;
    TClient* Client;
    std::string FullLogin;      // friend login
    std::string Name;
    std::string PublicKey;
    std::string ServerPublicKey;
    EFriendStatus Status;
    EConnectionStatus ConnectionStatus;
    bool AcceptingConnection;
    std::string Buffer;
    std::unique_ptr<NUdt::TClient> UdtClient;
};

typedef std::unordered_map<std::string, TFriend> TFriends;
typedef TFriends::iterator TFriendIterator;

} // NVocal
