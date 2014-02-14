#include <ctime>
#include <memory>
#include <vector>

#include <utils/exception.h>
#include <utils/cast.h>

#include <QCoreApplication>

#include <library/http_server/server.h>

#include "game_server.h"

class TServerPool {
public:
    TServerPool(const QString& hostAddr, ui16 firstPort, size_t count)
        : FirstPort(firstPort)
        , HostAddr(hostAddr)
    {
        assert(count < 100 && "server count too big");
        for (ui16 port = firstPort; port < firstPort + count; ++port) {
            std::unique_ptr<TGameServer> gameServer(new TGameServer(port));
            Servers.push_back(std::move(gameServer));
        }
    }
    ui16 FindBestServer() {
        for (size_t i = 0; i < Servers.size(); ++i) {
            if (!Servers[i]->Empty() && !Servers[i]->Full()) {
                return FirstPort + i;
            }
        }
        for (size_t i = 0; i < Servers.size(); ++i) {
            if (!Servers[i]->Full()) {
                return FirstPort + i;
            }
        }
        throw UException("server is full");
        return (ui16)-1;
    }
    optional<NHttpServer::TResponse> OnQuickRequest(const NHttpServer::TRequest&) {
        NHttpServer::TResponse response;
        ui16 bestServerPort = FindBestServer();
        response.Data = QString("%1:%2").arg(HostAddr).arg(bestServerPort).toStdString();
        return response;
    }
private:
    std::vector<std::unique_ptr<TGameServer> > Servers;
    ui16 FirstPort;
    QString HostAddr;
};

int main(int argc, char *argv[]) {
    try {
        srand(time(NULL));
        QCoreApplication a(argc, argv);

        TServerPool serverPool("127.0.0.1", 9999, 10);
        NHttpServer::TSettings settings(9999);
        NHttpServer::THttpServer server(settings);
        server.HandleAction("/quick", std::bind(&TServerPool::OnQuickRequest, &serverPool, std::placeholders::_1));

        return a.exec();
    } catch (const std::exception& e) {
        std::cerr << "Critical error: " << e.what() << "\n";
        return 42;
    }
}
