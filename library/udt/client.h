#pragma once

#include <utils/network_address.h>
#include <functional>
#include <memory>
#include <chrono>
#include <utils/types.h>
#include <utils/buffer.h>

namespace NUdt {

typedef std::function<void()> TFunction;
typedef std::function<void(bool /*success*/)> TConnectionCallback;
typedef std::function<void(const TBuffer& /*data*/)> TDataCallback;

struct TClientConfig {
    TClientConfig()
        : Timeout(10000)
    {
    }
    TConnectionCallback ConnectionCallback; // on connected (or failed to connect)
    TFunction ConnectionLostCallback;       // on connection lost
    TDataCallback DataReceivedCallback;     // on data received
    std::chrono::milliseconds Timeout;      // connection timeout
};

class TClientImpl;
class TClient {
public:
    TClient(const TClientConfig& config);
    ~TClient();
    void Connect(const TNetworkAddress& address, ui16 localPort = 0, bool overNat = false);
    TNetworkAddress GetLocalAddress();
    void Disconnect();
    void Send(const TBuffer& data);
private:
    std::unique_ptr<TClientImpl> Impl;

};

} // NUdt
