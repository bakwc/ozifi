#pragma once

#include <memory>
#include <functional>
#include <boost/asio/ip/address.hpp>
#include <utils/types.h>
#include <utils/network_address.h>

namespace NVocal {

// bool newConnection(ipAddr); - true to accept, false to decline
typedef std::function<bool(const TNetworkAddress& address)> TNewConnectionCallback;

struct TServerConfig {
    ui16 Port;
    size_t MaxConnections;
    TNewConnectionCallback NewConnectionCallback;
};

class TServerImpl;
class TServer {
public:
    TServer(const TServerConfig& config);
    ~TServer();
private:
    std::unique_ptr<TServerImpl> Impl;
};

} // NVocal
