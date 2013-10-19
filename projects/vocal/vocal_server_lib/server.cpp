#include <unordered_map>
#include <library/udt/server.h>
#include <library/http_server/server.h>

#include "server.h"
#include "storage.h"

using namespace std;
using namespace std::placeholders;

namespace NVocal {

enum EClientStatus {
    CS_Unknown,
    CS_Authorized
};

class TClient {
    TNetworkAddress Address;
    EClientStatus Status;
};

class TServerImpl {
public:
    TServerImpl(const TServerConfig& config)
        : Config(config)
    {
        NUdt::TServerConfig udtConfig;
        udtConfig.NewConnectionCallback = bind(&TServerImpl::OnClientConnected, this, _1);
        udtConfig.DataReceivedCallback = bind(&TServerImpl::OnDataReceived, this, _1, _2);
        udtConfig.Port = config.Port;
        Server.reset(new NUdt::TServer(udtConfig));
        NHttpServer::TSettings httpConfig(config.AdminPort);
        httpConfig.Threads = 5;
        HttpServer.reset(new NHttpServer::THttpServer(httpConfig));
        HttpServer->HandleAction("/change_hostname", bind(&TServerImpl::OnChangeHostname, this, _1));
    }
    void PrintStatus(std::ostream& out) {
    }
    void PrintClientStatus(const std::string& client, std::ostream& out) {
    }
private:
    bool OnClientConnected(const TNetworkAddress& client) {
    }
    void OnDataReceived(const TBuffer& data, const TNetworkAddress& client) {
    }
    boost::optional<NHttpServer::TResponse> OnChangeHostname(const NHttpServer::TRequest& request) {
    }
private:
     TServerConfig Config;
     unique_ptr<NUdt::TServer> Server;
     unique_ptr<NHttpServer::THttpServer> HttpServer;
     unique_ptr<TClientInfoStorage> ClientInfoStorage;
     unique_ptr<TMessageStorage> MessageStorage;
};

TServer::TServer(const TServerConfig& config)
    : Impl(new TServerImpl(config))
{
}

TServer::~TServer() {
}

void TServer::PrintStatus(std::ostream& out) {
    Impl->PrintStatus(out);
}

void TServer::PrintClientStatus(const std::string& client, std::ostream& out) {
    Impl->PrintClientStatus(client, out);
}

}
