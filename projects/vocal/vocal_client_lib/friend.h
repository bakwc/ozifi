#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include <unordered_set>

#include <boost/optional.hpp>
#include <library/udt/client.h>
#include <library/udt/server.h>

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

inline bool IsOnline(EFriendStatus status) {
    return status == FS_Available ||
           status == FS_Away ||
           status == FS_Busy;
}

inline bool IsAuthorized(EFriendStatus status) {
    return status == FS_Offline || IsOnline(status);
}

enum EConnectionStatus {
    COS_Offline,
    COS_ConnectingToServer,
    COS_WaitingFriendAddress,
    COS_ConnectingToFriend,
    COS_WaitingFriendConnection,
    COS_AcceptedConnection,
    COS_Connected
};

enum EFriendPacketType {
    FPT_RandomSequence,
    FPT_RandomSequenceConfirm,
    FPT_Encrypted,
    FPT_Authorized,
    FPT_Message
};

class TClient;
class TFriend {
    friend class TClient;
public:
    TFriend(TClient* client);
    TFriend(TClient* client, const std::string& login, EFriendStatus status);
    const std::string& GetLogin();
    const std::string& GetName();
    EFriendStatus GetStatus();
    const std::string& GetStatusMessage();
    std::vector<TMessage> GetHistory(); // old messages
    void SendMessage(const std::string& text);
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
    void OnOfflineMessageReceived(const TBuffer& data, bool isIncoming);
    void OnMessageReceived(const TMessage& message);
private:
    void InitUdtClient();
    bool OnClientConnected(const TNetworkAddress& addr);
    void OnConnectionEstablished();
    void SendMessage(const TMessage& message);
    void SendEncrypted(const TBuffer& data, EFriendPacketType friendPacketType);
    void SendSerialized(const TBuffer& data, EFriendPacketType friendPacketType);
    void SendRaw(const TBuffer& data);
    void UpdateOnlineStatus();
protected:
    bool ToDelete;
    TClient* Client;
    std::string FullLogin;      // friend login
    std::string Name;
    std::string PublicKey;
    std::string ServerPublicKey;
    EFriendStatus Status;
    EConnectionStatus ConnectionStatus;
    bool SelfAuthorized;    // friend authorize us
    bool FriendAuthorized;  // we authorized friend
    bool AcceptingConnection;
    std::string Buffer;
    std::unique_ptr<NUdt::TClient> UdtClient;
    std::unique_ptr<NUdt::TServer> UdtServer;
    TNetworkAddress FriendAddress;
    ui16 LocalPort;
    ui16 PublicPort;
    TNetworkAddress PublicAddress;
    std::string RandomSequence;
    std::string SessionKey;
    std::string FriendRandomSequence;
    std::string FriendSessionKey;
    std::string SelfSessionKey;
    std::string SelfOfflineKey;     // using for encryption
    std::string FriendOfflineKey;   // using for decryption
    std::unordered_set<std::string> PrevMessages;
};

typedef std::shared_ptr<TFriend> TFriendRef;

typedef std::unordered_map<std::string, TFriendRef> TFriends;
typedef TFriends::iterator TFriendIterator;

} // NVocal
