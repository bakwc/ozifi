#pragma once

#include <memory>
#include <functional>
#include <boost/asio/buffer.hpp>
#include <utils/types.h>
#include <utils/network_address.h>
#include <utils/buffer.h>

namespace NUdt {

// bool newConnection(ipAddr); - true to accept, false to decline
typedef std::function<bool(const TNetworkAddress& /*address*/)> TNewConnectionCallback;
typedef std::function<void(const TNetworkAddress& /*address*/)> TAddressedCallback;
typedef std::function<void(const TNetworkAddress& /*address*/)> TConnectionAcceptedCallback;
typedef std::function<void(const TBuffer& /*data*/, const TNetworkAddress& /*address*/)> TAddressedDataCallback;

struct TServerConfig {
    TServerConfig();
    ui16 Port;
    size_t MaxConnections;
    TNewConnectionCallback NewConnectionCallback;               // on new client connected
    TAddressedCallback ConnectionLostCallback;                  // on client disconnected
    TConnectionAcceptedCallback ConnectionAcceptedCallback;     // on connection accepted
    TAddressedDataCallback DataReceivedCallback;                // on data received
};

class TServerImpl;
class TServer {
public:
    TServer(const TServerConfig& config);
    ~TServer();
    void Send(const TBuffer& data, const TNetworkAddress& address);
    void DisconnectClient(const TNetworkAddress& client);
    ui16 GetPort();
private:
    std::unique_ptr<TServerImpl> Impl;
};

} // NUdt
