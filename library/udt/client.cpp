#include "client.h"

#include <thread>
#include <mutex>
#include <boost/optional.hpp>
#include <contrib/udt4/udt.h>
#include <utils/exception.h>


using namespace std;

namespace NUdt {

enum EClientStatus {
    CS_Disconnected,
    CS_Connecting,
    CS_Connected
};

class TClientImpl { // todo: implement this
public:
    TClientImpl(const TClientConfig& config)
        : Config(config)
        , Status(CS_Disconnected)
    {
        Socket = UDT::socket(AF_INET, SOCK_STREAM, 0);
    }
    ~TClientImpl() {
        Disconnect();
        if (WorkerThreadHolder) {
            WorkerThreadHolder->join();
        }
    }

    inline void Connect(const TNetworkAddress& address, bool overNat) {
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
        CurrentConnection = address;

        // todo: set timeout from config
        MainEid = UDT::epoll_create();
        SetAsyncMode(true);
        UDT::epoll_add_usock(MainEid, Socket);
        Status = CS_Connecting;
        auto duration = chrono::system_clock::now().time_since_epoch();
        LastActive = chrono::duration_cast<chrono::milliseconds>(duration);
        UDT::connect(Socket, address.Sockaddr(), address.SockaddrLength());
        Done = false;
        WorkerThreadHolder.reset(new thread(std::bind(&TClientImpl::WorkerThread, this)));
    }
    inline void Disconnect(bool waitWorkerThread = true) {
        // todo: ensure that ondisconnected callback will be called
        if (Status == CS_Disconnected) {
            return;
        }
        Status = CS_Disconnected;
        Done = true;
        //SetAsyncMode(false);
        //UDT::close(Socket);  todo: close socket if required
        UDT::epoll_release(MainEid);
        Config.ConnectionLostCallback();
    }

    inline void Send(const TBuffer& data) {
        if (Status != CS_Connected) {
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
            std::set<UDTSOCKET> eventedSockets;
            // when connecting = waiting for write events too
            std::set<UDTSOCKET>* writeEvents = Status == CS_Connecting ? &eventedSockets : 0;
            UDT::epoll_wait(MainEid, &eventedSockets, writeEvents, 5000);
            for (set<UDTSOCKET>::iterator it = eventedSockets.begin(); it != eventedSockets.end(); ++it) {
                if (*it == Socket && Status == CS_Connecting) {
                    Status = CS_Connected;
                    // todo: get and store local address
                    Config.ConnectionCallback(true);
                } else {
                    int result = UDT::recv(*it, buff.data(), 1024, 0);
                    if (UDT::ERROR != result) {
                        Config.DataReceivedCallback(TBuffer(buff.data(), result));
                    } else {
                        // todo: process connection lost and other
                    }
                }
            }
            if (Status == CS_Connecting) {
                auto duration = chrono::system_clock::now().time_since_epoch();
                chrono::milliseconds currentTime = chrono::duration_cast<chrono::milliseconds>(duration);
                if (currentTime - LastActive > Config.Timeout) {
                    Disconnect(false);
                    Config.ConnectionCallback(false);
                    return;
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
    EClientStatus Status;
    chrono::milliseconds LastActive;
    int MainEid;
};

TClient::TClient(const TClientConfig& config)
    : Impl(new TClientImpl(config))
{
}

TClient::~TClient() {
}

void TClient::Connect(const TNetworkAddress& address, bool overNat) {
    Impl->Connect(address, overNat);
}

void TClient::Disconnect() {
    Impl->Disconnect();
}

void TClient::Send(const TBuffer& data) {
    Impl->Send(data);
}

}
