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
    inline void Connect(const TNetworkAddress& address, bool overNat, TConnectionCallback callback) {
        if (CurrentConnection.is_initialized()) {
            SetAsyncMode(false);
            UDT::close(Socket); // todo: process connection close error
            CurrentConnection = boost::optional<TNetworkAddress>();
            Done = true;
            WorkerThreadHolder->join();
        }
        if (overNat) {
            if (!CurrentLocalAddress.is_initialized()) {
                throw UException("cannot traverse nat cause local address not available");
            }
            // todo: bind socket using CurrentLocalAddress
        }
        SetAsyncMode(true);
        // todo: connect to server using address
        Done = false;
        WorkerThreadHolder.reset(new thread(std::bind(&TClientImpl::WorkerThread, this)));
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
        // todo: process receiving and connection drops
    }

private:
    TClientConfig Config;
    boost::optional<TNetworkAddress> CurrentConnection;
    boost::optional<TNetworkAddress> CurrentLocalAddress;
    UDTSOCKET Socket;
    unique_ptr<thread> WorkerThreadHolder;
    mutex Lock;
    bool Done;
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

void TClient::Send(const TBuffer& data) {
    Impl->Send(data);
}

}
