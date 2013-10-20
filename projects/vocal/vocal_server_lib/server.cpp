#include <projects/vocal/vocal_lib/resolver.h>

#include "server.h"

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

TServer::TServer(const TServerConfig& config)
    : Config(config)
{
    ClientInfoStorage.reset(new TClientInfoStorage(config.DataDirectory + "/clients"));
    MessageStorage.reset(new TMessageStorage(config.DataDirectory + "/messages"));
    SelfStorage.reset(new TSelfStorage(config.DataDirectory + "/self"));
    NUdt::TServerConfig udtConfig;
    udtConfig.NewConnectionCallback = bind(&TServer::OnClientConnected, this, _1);
    udtConfig.DataReceivedCallback = bind(&TServer::OnDataReceived, this, _1, _2);
    udtConfig.Port = config.Port;
    Server.reset(new NUdt::TServer(udtConfig));
    NHttpServer::TSettings httpConfig(config.AdminPort);
    httpConfig.Threads = 5;
    HttpServer.reset(new NHttpServer::THttpServer(httpConfig));
}

void TServer::PrintStatus(std::ostream& out) {
}

void TServer::PrintClientStatus(const std::string& client, std::ostream& out) {
}

bool TServer::OnClientConnected(const TNetworkAddress& client) {
}

void TServer::OnDataReceived(const TBuffer& data, const TNetworkAddress& client) {
}

}
