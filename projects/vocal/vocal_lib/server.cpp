#ifdef __WIN32__
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif
#include <thread>
#include <functional>
#include <unordered_map>
#include <mutex>
#include <contrib/udt4/udt.h>
#include <utils/exception.h>

#include "server.h"

using namespace std;
using namespace std::placeholders;

namespace NVocal {

typedef unordered_map<TNetworkAddress, UDTSOCKET> TClients;

class TServerImpl {
public:
    TServerImpl(const TServerConfig& config)
        : Config(config)
        , Done(false)
    {
        Server = UDT::socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(config.Port);
        addr.sin_addr.s_addr = INADDR_ANY;
        memset(&(addr.sin_zero), '\0', 8);
        if (UDT::ERROR == UDT::bind(Server, (sockaddr*)&addr, sizeof(addr))) {
            throw UException(string("failed to start server: ") + UDT::getlasterror().getErrorMessage());
        }
        bool block = false;
        if (UDT::ERROR == UDT::setsockopt(Server, 0, UDT_SNDSYN, &block, sizeof(bool))) {
            throw UException(string("failed to star server: ") + UDT::getlasterror().getErrorMessage());
        }
        if (UDT::ERROR == UDT::setsockopt(Server, 0, UDT_RCVSYN, &block, sizeof(bool))) {
            throw UException(string("failed to star server: ") + UDT::getlasterror().getErrorMessage());
        }
        if (UDT::ERROR == UDT::listen(Server, config.MaxConnections)) {
            throw UException(string("failed to star server: ") + UDT::getlasterror().getErrorMessage());
        }
        MainEid = UDT::epoll_create();
        WorkerThreadHolder.reset(new thread(std::bind(&TServerImpl::WorkerThread, this)));
        ConnectionsThreadHolder.reset(new thread(std::bind(&TServerImpl::ConnectionsThread, this)));
    }
    ~TServerImpl() {
        Done = true;
        WorkerThreadHolder->join();
        ConnectionsThreadHolder->join();
        UDT::epoll_release(MainEid);
        UDT::close(Server);
    }

    void WorkerThread() {
        while (!Done) {
            set<UDTSOCKET> eventedSockets;
            UDT::epoll_wait(MainEid, &eventedSockets, &eventedSockets, 200);
            for (set<UDTSOCKET>::iterator it = eventedSockets.begin(); it != eventedSockets.end(); ++it) {
                // todo: process sockets
            }
        }
    }
    void ConnectionsThread() {
        int eid = UDT::epoll_create();
        UDT::epoll_add_usock(eid, Server);
        while (!Done) {
            set<UDTSOCKET> eventedSockets;
            UDT::epoll_wait(eid, &eventedSockets, &eventedSockets, 1000);
            if (eventedSockets.size() > 0) {
                sockaddr_in clientAddr;
                int clientAddrLen;
                UDTSOCKET clientSocket = UDT::accept(Server, (sockaddr*)&clientAddr, &clientAddrLen);
                if (clientSocket != UDT::INVALID_SOCK) {
                    TNetworkAddress addrs(*(ui32*)clientAddr.sin_addr.s_addr, clientAddr.sin_port);
                    if (Config.NewConnectionCallback(addrs)) {
                        lock_guard<mutex> guard(Lock);
                        Clients[addrs] = clientSocket;
                        UDT::epoll_add_usock(MainEid, clientSocket);
                    } else {
                        UDT::close(clientSocket);
                    }

                }
            }
        }
        UDT::epoll_release(eid);
    }
private:
    TServerConfig Config;
    UDTSOCKET Server;
    TClients Clients;
    unique_ptr<thread> WorkerThreadHolder;
    unique_ptr<thread> ConnectionsThreadHolder;
    bool Done;
    mutex Lock;
    int MainEid;
};


TServer::TServer(const TServerConfig& config)
    : Impl(new TServerImpl(config))
{
}

TServer::~TServer()
{
}


}
