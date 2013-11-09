#pragma once

#include <string>
#include <memory>
#include <iostream>
#include <unordered_map>
#include <queue>
#include <mutex>
#include <utils/types.h>
#include <library/udt/server.h>
#include <library/udt/client.h>
#include <library/http_server/server.h>

#include "storage.h"

namespace NVocal {

struct TServerConfig {
    ui16 Port;                  // local port
    ui16 AdminPort;             // web admin port
    std::string Hostname;       // hostname with correct srv records
    std::string DataDirectory;  // directory for database
};

enum EClientStatus {
    CS_Unknown,
    CS_Registering,
    CS_Logining,
    CS_Authorizing,
    CS_Authorized
};

class TClient {
public:
    TClient(const TNetworkAddress& address);
    bool AcquireSyncLock(); // false if already syncing
    bool ReleaseSyncLock(); // false if required to sync again
                            // (there were sync request while we were syncing
public:
    TNetworkAddress Address;
    EClientStatus Status;
    TClientInfo Info;
    std::string Login;
    std::string CaptchaText;
    std::string Buffer;
    std::string RandomSequence;
    std::string SessionKey;
    TDuration SessionLastSync;
private:
    std::mutex Lock;
    bool Syncing;
    bool NeedToResync;
};
typedef std::shared_ptr<TClient> TClientRef;

typedef std::function<void(const TBuffer& /*data*/, const std::string& /*host*/)> TPartnerDataCallback;
class TPartnerServer {
public:
    const size_t MAX_QUEUE_SIZE = 100;
    enum EStatus {
        ST_Disconnected,
        ST_Connecting,
        ST_Connected
    };
    TPartnerServer(const std::string& host, TPartnerDataCallback& onDataReceived);
    ~TPartnerServer();
    void Send(const std::string& message);
private:
    void OnConnected(bool success);
private:
    std::queue<std::string> Messages;
    TPartnerDataCallback OnDataReceived;
    std::unique_ptr<NUdt::TClient> Client;
    EStatus Status;
    // todo: mutex here
};
typedef std::shared_ptr<TPartnerServer> TPartnerServerRef;

class TServer {
public:
    TServer(const TServerConfig& config);
    void PrintStatus(std::ostream& out);
    void PrintClientStatus(const std::string& client, std::ostream& out);
private:
    bool OnClientConnected(const TNetworkAddress& addr);
    void OnDataReceived(const TBuffer& data, const TNetworkAddress& addr);
    void OnServerDataReceived(const TBuffer& data, const std::string& host);
    void SendAddFriendRequest(const std::string& login,
                              const std::string& pubKey,
                              const std::string& frndLogin);
    void OnAddFriendRequest(const std::string& login, const string& frndLogin);
    void SendToServer(const std::string& host, const std::string& message);
    void SyncMessages(const std::string& login, TDuration from, TDuration to); // thread-safe
    void SyncNewMessages(const std::string& login); // thread-safe
    void UpdateClientInfo(const TClientInfo& info);
private:
     TServerConfig Config;
     unique_ptr<NUdt::TServer> Server;
     unique_ptr<NHttpServer::THttpServer> HttpServer;
     unique_ptr<TClientInfoStorage> ClientInfoStorage;
     unique_ptr<TSelfStorage> SelfStorage;
     unique_ptr<TMessageStorage> MessageStorage;
     unordered_map<TNetworkAddress, TClientRef> Clients;
     unordered_map<std::string, TPartnerServerRef> Servers;
     unordered_map<std::string, TClientRef> ClientsByLogin;
};

}
