#include <boost/optional.hpp>

#include "client.h"

namespace NUdt {

class TClientImpl { // todo: implement this
public:
    TClientImpl(const TClientConfig& config)
        : Config(config)
    {
    }
    inline void Connect(const TNetworkAddress& address, TConnectionCallback callback) {
    }
    inline void Send(const TBuffer& data) {
    }
private:
    TClientConfig Config;
    boost::optional<TNetworkAddress> CurrentConnection;
};

TClient::TClient(const TClientConfig& config)
    : Impl(new TClientImpl(config))
{
}

TClient::~TClient() {
}

void TClient::Connect(const TNetworkAddress& address, TConnectionCallback callback) {
    Impl->Connect(address, callback);
}

void TClient::Send(const TBuffer& data) {
    Impl->Send(data);
}

}
