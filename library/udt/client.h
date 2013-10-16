#pragma once

#include <functional>
#include <memory>
#include <utils/types.h>
#include <utils/network_address.h>
#include <utils/buffer.h>

namespace NUdt {

typedef std::function<void()> TFunction;
typedef std::function<void(bool /*success*/)> TConnectionCallback;
typedef std::function<void(const TBuffer& /*data*/)> TDataCallback;

struct TClientConfig {
    TConnectionCallback ConnectionCallback; // on connected (or failed to connect)
    TFunction ConnectionLostCallback;       // on connection lost
    TDataCallback DataReceiveCallback;      // on data received
};

class TClientImpl;
class TClient {
public:
    TClient(const TClientConfig& config);
    ~TClient();
    void Connect(const TNetworkAddress& address, bool overNat, TConnectionCallback callback);
    void Send(const TBuffer& data);
private:
    std::unique_ptr<TClientImpl> Impl;
};

} // NUdt
