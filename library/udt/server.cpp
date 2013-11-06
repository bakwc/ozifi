#ifdef __WIN32__
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif
#include <thread>
#include <functional>
#include <unordered_map>
#include <mutex>
#include <boost/asio/detail/array.hpp>
#include <contrib/udt4/udt.h>
#include <utils/exception.h>

#include "server.h"

using namespace std;
using namespace std::placeholders;

namespace NUdt {

struct TClient {
    TClient(UDPSOCKET socket, const TNetworkAddress& address)
        : Socket(socket)
        , Address(address)
    {
    }
    UDTSOCKET Socket;
    TNetworkAddress Address;
};

typedef shared_ptr<TClient> TClientRef;

typedef unordered_map<TNetworkAddress, TClientRef> TClientsByAddr;
typedef unordered_map<UDTSOCKET, TClientRef> TClientsBySocket;

class TClients {
public:
    // todo: throw exceptions instead of assert.. or not
    inline void Insert(TClientRef cli) {
        assert(ClientsBySock.find(cli->Socket) == ClientsBySock.end() && "client already exists");
        ClientsByAddr[cli->Address] = cli;
        ClientsBySock[cli->Socket] = cli;
    }
    inline TClientRef Get(UDTSOCKET socket) {
        assert(ClientsBySock.find(socket) != ClientsBySock.end() && "client not found");
        return ClientsBySock[socket];
    }
    inline TClientRef Get(const TNetworkAddress& address) {
        assert(ClientsByAddr.find(address) != ClientsByAddr.end() && "client not found");
        return ClientsByAddr[address];
    }
    inline void Remove(UDTSOCKET socket) {
        assert(ClientsBySock.find(socket) != ClientsBySock.end() && "client not found");
        TClientRef client = ClientsBySock[socket];
        ClientsBySock.erase(client->Socket);
        ClientsByAddr.erase(client->Address);
    }
private:
    TClientsByAddr ClientsByAddr;
    TClientsBySocket ClientsBySock;
};

class TServerImpl {
public:
    TServerImpl(const TServerConfig& config)
        : Config(config)
        , Done(false)
    {
        Socket = UDT::socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(config.Port);
        addr.sin_addr.s_addr = INADDR_ANY;
        memset(&(addr.sin_zero), '\0', 8);
        if (UDT::ERROR == UDT::bind(Socket, (sockaddr*)&addr, sizeof(addr))) {
            throw UException(string("udt: failed to bind socket: ") + UDT::getlasterror().getErrorMessage());
        }
        bool block = false;
        if (UDT::ERROR == UDT::setsockopt(Socket, 0, UDT_SNDSYN, &block, sizeof(bool))) {
            throw UException(string("udt: failed to disable SNDSYN: ") + UDT::getlasterror().getErrorMessage());
        }
        if (UDT::ERROR == UDT::setsockopt(Socket, 0, UDT_RCVSYN, &block, sizeof(bool))) {
            throw UException(string("udt: failed to disable SCVSYN: ") + UDT::getlasterror().getErrorMessage());
        }
        if (UDT::ERROR == UDT::listen(Socket, config.MaxConnections)) {
            throw UException(string("udt: failed to listen: ") + UDT::getlasterror().getErrorMessage());
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
        UDT::close(Socket);
    }
    bool Send(const TBuffer& data, const TNetworkAddress& address) {
        UDTSOCKET sock;
        {
            lock_guard<mutex> guard(Lock);
            sock = Clients.Get(address)->Socket;
        }
        int result = UDT::send(sock, data.Data(), data.Size(), 0);
        if (result == UDT::ERROR) {
            UDT::ERRORINFO error = UDT::getlasterror();
            if (error.getErrorCode() == UDT::ERRORINFO::ECONNLOST) {
                // todo: process disconnected client
            }
            return false;
        }
        assert(data.Size() == result && "sended not all bytes");
        return true;
    }
    void DisconnectClient(const TNetworkAddress& client) {
        // todo: drop client
    }
    void WorkerThread() {
        boost::asio::detail::array<char, 1024> buff;
        while (!Done) {
            set<UDTSOCKET> eventedSockets;
            UDT::epoll_wait(MainEid, &eventedSockets, NULL, 200);
            for (set<UDTSOCKET>::iterator it = eventedSockets.begin(); it != eventedSockets.end(); ++it) {
                int result = UDT::recv(*it, buff.data(), 1024, 0);
                if (UDT::ERROR != result) {
                    TNetworkAddress clientAddr;
                    {
                        lock_guard<mutex> guard(Lock);
                        clientAddr = Clients.Get(*it)->Address;
                    }
                    Config.DataReceivedCallback(TBuffer(buff.data(), result), clientAddr);
                } else {
                    // todo: process connection lost and other
                }
            }
        }
    }
    void ConnectionsThread() {
        int eid = UDT::epoll_create();
        UDT::epoll_add_usock(eid, Socket);
        while (!Done) {
            set<UDTSOCKET> eventedSockets;
            int res = UDT::epoll_wait(eid, &eventedSockets, NULL, 1000);
            if (eventedSockets.size() > 0) {
                sockaddr_in clientAddr;
                int clientAddrLen;
                UDTSOCKET clientSocket = UDT::accept(Socket, (sockaddr*)&clientAddr, &clientAddrLen);
                if (clientSocket != UDT::INVALID_SOCK) {
                    TNetworkAddress addr(*(ui32*)(&clientAddr.sin_addr.s_addr), clientAddr.sin_port);
                    if (Config.NewConnectionCallback(addr)) {
                        {
                            lock_guard<mutex> guard(Lock);
                            Clients.Insert(make_shared<TClient>(clientSocket, addr));
                        }
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
    UDTSOCKET Socket;
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

void TServer::Send(const TBuffer& data, const TNetworkAddress& address) {
    Impl->Send(data, address);
}

void TServer::DisconnectClient(const TNetworkAddress& client) {
    Impl->DisconnectClient(client);
}

TServer::~TServer()
{
}


} // NUdt
