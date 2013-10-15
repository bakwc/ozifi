#pragma once

#include <memory>
#include <functional>
#include <boost/asio/buffer.hpp>
#include <utils/types.h>
#include <utils/network_address.h>
#include <utils/buffer.h>

namespace NUdt {

// bool newConnection(ipAddr); - true to accept, false to decline
typedef std::function<bool(const TNetworkAddress& address)> TNewConnectionCallback;
typedef std::function<void(const TBuffer& data, const TNetworkAddress& address)> TDataReceivedCallback;

struct TServerConfig {
    ui16 Port;
    size_t MaxConnections;
    TNewConnectionCallback NewConnectionCallback; // on new client connected
    TDataReceivedCallback DataReceivedCallback;    // on data received
};

class TServerImpl;
class TServer {
public:
    TServer(const TServerConfig& config);
    void Send(const TBuffer& data, const TNetworkAddress& address);
    ~TServer();
private:
    std::unique_ptr<TServerImpl> Impl;
};

} // NUdt
