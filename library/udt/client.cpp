#include <thread>
#include <mutex>
#include <boost/optional.hpp>
#include <utils/exception.h>

#include "client.h"

#include <contrib/udt4/udt.h>

using namespace std;

namespace NUdt {

class TClientImpl { // todo: implement this
public:
    TClientImpl(const TClientConfig& config)
        : Config(config)
    {
        Socket = UDT::socket(AF_INET, SOCK_STREAM, 0);
    }
    ~TClientImpl() {
        Done = true;
        WorkerThreadHolder->join();
        UDT::epoll_release(MainEid);
    }

    inline void Connect(const TNetworkAddress& address, bool overNat, TConnectionCallback callback) {
        if (CurrentConnection.is_initialized()) {
            SetAsyncMode(false);
            UDT::close(Socket); // todo: process connection close error
            CurrentConnection = boost::optional<TNetworkAddress>();
            Done = true;
            WorkerThreadHolder->join();
            UDT::epoll_release(MainEid);
        }
        if (overNat) {
            if (!CurrentLocalAddress.is_initialized()) {
                throw UException("cannot traverse nat cause local address not available");
            }
            UDT::bind(Socket, CurrentLocalAddress->Sockaddr(), CurrentLocalAddress->SockaddrLength());
        }
        SetAsyncMode(true);
        UDT::connect(Socket, address.Sockaddr(), address.SockaddrLength());
        MainEid = UDT::epoll_create();
        UDT::epoll_add_usock(MainEid, Socket);
        Done = false;
        WorkerThreadHolder.reset(new thread(std::bind(&TClientImpl::WorkerThread, this)));
    }
    inline void Disconnect() {
        // todo: ensure that ondisconnected callback will be called
        UDT::close(Socket);
    }

    inline void Send(const TBuffer& data) {
        if (!CurrentConnection) {
            throw UException("not connected");
        }
        UDT::send(Socket, data.Data(), data.Size(), 0);
    }
private:
    inline void SetAsyncMode(bool async) {
        if (UDT::ERROR == UDT::setsockopt(Socket, 0, UDT_SNDSYN, &async, sizeof(bool))) {
            throw UException(string("failed to set async mode: ") + UDT::getlasterror().getErrorMessage());
        }
        if (UDT::ERROR == UDT::setsockopt(Socket, 0, UDT_RCVSYN, &async, sizeof(bool))) {
            throw UException(string("failed to set async mode: ") + UDT::getlasterror().getErrorMessage());
        }
    }

    void WorkerThread() {
        boost::asio::detail::array<char, 1024> buff;
        while (!Done) {
            set<UDTSOCKET> eventedSockets;
            UDT::epoll_wait(MainEid, &eventedSockets, &eventedSockets, 200);
            for (set<UDTSOCKET>::iterator it = eventedSockets.begin(); it != eventedSockets.end(); ++it) {
                int result = UDT::recv(*it, buff.data(), 1024, 0);
                if (UDT::ERROR != result) {
                    Config.DataReceivedCallback(TBuffer(buff.data(), result));
                } else {
                    // todo: process connection lost and other
                }
            }
        }
    }

private:
    TClientConfig Config;
    boost::optional<TNetworkAddress> CurrentConnection;
    boost::optional<TNetworkAddress> CurrentLocalAddress;
    UDTSOCKET Socket;
    unique_ptr<thread> WorkerThreadHolder;
    mutex Lock;
    bool Done;
    int MainEid;
};

TClient::TClient(const TClientConfig& config)
    : Impl(new TClientImpl(config))
{
}

TClient::~TClient() {
}

void TClient::Connect(const TNetworkAddress& address, bool overNat, TConnectionCallback callback) {
    Impl->Connect(address, overNat, callback);
}

void TClient::Disconnect() {
    Impl->Disconnect();
}

void TClient::Send(const TBuffer& data) {
    Impl->Send(data);
}

}
