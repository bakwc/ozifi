#pragma once

#include <string>
#include <memory>
#include <iostream>
#include <unordered_map>
#include <utils/types.h>
#include <library/udt/server.h>
#include <library/http_server/server.h>

#include "storage.h"

namespace NVocal {

struct TServerConfig {
    ui16 Port;                  // local port
    ui16 AdminPort;             // web admin port
    std::string Hostname;       // hostname with correct srv records
    std::string DataDirectory;  // directory for database
};

class TServer {
public:
    TServer(const TServerConfig& config);
    void PrintStatus(std::ostream& out);
    void PrintClientStatus(const std::string& client, std::ostream& out);
private:
    bool OnClientConnected(const TNetworkAddress& client);
    void OnDataReceived(const TBuffer& data, const TNetworkAddress& client);
private:
     TServerConfig Config;
     unique_ptr<NUdt::TServer> Server;
     unique_ptr<NHttpServer::THttpServer> HttpServer;
     unique_ptr<TClientInfoStorage> ClientInfoStorage;
     unique_ptr<TSelfStorage> SelfStorage;
     unique_ptr<TMessageStorage> MessageStorage;
};

}
