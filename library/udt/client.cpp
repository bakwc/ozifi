#include "client.h"

#include <thread>
#include <mutex>
#include <boost/optional.hpp>
#include <contrib/udt4/udt.h>
#include <utils/exception.h>

#include "event_queue.h"

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

    inline TNetworkAddress GetLocalAddress() {
        struct sockaddr_in localaddr;
        int len;
        if (UDT::ERROR == UDT::getsockname(Socket, (sockaddr*)&localaddr, &len)) {
            throw UException(string("failed to get local addr: ") + UDT::getlasterror().getErrorMessage());
        }
        return localaddr;
    }

    inline void Connect(TNetworkAddress address, ui16 localPort, bool overNat) {
        if (CurrentConnection.is_initialized()) {
            if (localPort == 0 && overNat) {
                localPort = GetLocalAddress().Port;
            }
            SetAsyncMode(false);
            UDT::close(Socket); // todo: process connection close error
            Socket = UDT::socket(AF_INET, SOCK_STREAM, 0);
            CurrentConnection = boost::optional<TNetworkAddress>();
            Done = true;
            UDT::epoll_release(MainEid);
        }

        if (overNat) {
            if (localPort == 0) {
                throw UException("you need to specify local port to traverse nat");
            }
            sockaddr_in addr;
            addr.sin_family = AF_INET;
            addr.sin_port = htons(localPort);
            addr.sin_addr.s_addr = INADDR_ANY;
            memset(&(addr.sin_zero), '\0', 8);
            bool rendezvous = true;
            UDT::setsockopt(Socket, 0, UDT_RENDEZVOUS, &rendezvous, sizeof(bool));
            if (UDT::ERROR == UDT::bind(Socket, (sockaddr*)&addr, sizeof(addr))) {
                throw UException(string("cannot traverse nat: failed to bind: ") + UDT::getlasterror().getErrorMessage());
            }
        }
        CurrentConnection = address;

        // todo: set timeout from config
        MainEid = UDT::epoll_create();
        SetAsyncMode(true);
        UDT::epoll_add_usock(MainEid, Socket);
        Status = CS_Connecting;
        auto duration = chrono::system_clock::now().time_since_epoch();
        LastActive = chrono::duration_cast<chrono::milliseconds>(duration);
        if (UDT::ERROR == UDT::connect(Socket, address.Sockaddr(), address.SockaddrLength())) {
            throw UException(string("connect error: ") + UDT::getlasterror().getErrorMessage());
        }
        Done = true;
        while (WorkerThreadActive) {
            this_thread::sleep_for(chrono::milliseconds(50));
        }
        if (WorkerThreadHolder && WorkerThreadHolder->joinable()) {
            WorkerThreadHolder->join();
            WorkerThreadHolder.reset(nullptr);
        }
        if (!WorkerThreadHolder) {
            WorkerThreadHolder.reset(new thread(std::bind(&TClientImpl::WorkerThread, this)));
        }
        Done = false;
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
        if (Config.ConnectionLostCallback) {
            Config.ConnectionLostCallback();
        } else {
            cerr << "warning: connection lost callback missing\n";
        }
    }

    inline void Send(const TBuffer& data) {
        if (Status != CS_Connected) {
            throw UException("not connected");
        }
        for (size_t i = 0; i <= (data.Size() - 1) / 900; ++i) { // dirty hack to send big messages. otherwise udt fails
            if (UDT::ERROR == UDT::send(Socket, data.Data() + i * 900,
                                        std::min(data.Size() - i * 900, (size_t)900), 0))
            {
                throw UException(string("send error: ") + UDT::getlasterror().getErrorMessage());
            }
        }
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
        WorkerThreadActive = true;
        std::string buff;
        while (!Done) {
            std::set<UDTSOCKET> eventedSockets;
            // when connecting = waiting for write events too
            std::set<UDTSOCKET>* writeEvents = Status == CS_Connecting ? &eventedSockets : 0;
            UDT::epoll_wait(MainEid, &eventedSockets, writeEvents, 100);
            for (set<UDTSOCKET>::iterator it = eventedSockets.begin(); it != eventedSockets.end(); ++it) {
                if (*it == Socket && Status == CS_Connecting) {
                    Status = CS_Connected;
                    // todo: get and store local address
                    auto callback = Config.ConnectionCallback;
                    TEventQueue::Add([callback]() {
                        callback(true);
                    });
                } else {
                    buff.resize(1024);
                    int result = UDT::recv(*it, &buff[0], buff.size(), 0);
                    if (UDT::ERROR != result) {
                        auto callback = Config.DataReceivedCallback;
                        buff.resize(result);
                        TEventQueue::Add([callback, buff]() {
                            callback(TBuffer(buff));
                        });
                    } else {
                        if (UDT::getlasterror().getErrorCode() == 5004 ||
                            UDT::getlasterror().getErrorCode() == 2001)
                        {
                            UDT::close(*it);
                            Disconnect();
                        } else {
                            cerr << "warning: unhandled event\n";
                        }
                    }
                }
            }
            if (Status == CS_Connecting) {
                auto duration = chrono::system_clock::now().time_since_epoch();
                chrono::milliseconds currentTime = chrono::duration_cast<chrono::milliseconds>(duration);
                if (currentTime - LastActive > Config.Timeout) {
                    Disconnect(false);
                    auto callback = Config.ConnectionCallback;
                    TEventQueue::Add([callback] {
                        callback(false);
                    });
                    return;
                }
            }
        }
        WorkerThreadActive = false;
    }

private:
    TClientConfig Config;
    boost::optional<TNetworkAddress> CurrentConnection;
    UDTSOCKET Socket;
    unique_ptr<thread> WorkerThreadHolder;
    mutex Lock;
    bool Done = false;
    bool WorkerThreadActive = false;
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

void TClient::Connect(const TNetworkAddress& address, ui16 localPort, bool overNat) {
    Impl->Connect(address, localPort, overNat);
}

TNetworkAddress TClient::GetLocalAddress() {
    return Impl->GetLocalAddress();
}

void TClient::Disconnect() {
    Impl->Disconnect();
}

void TClient::Send(const TBuffer& data) {
    Impl->Send(data);
}

}
