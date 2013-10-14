#pragma once

#include <memory>
#include <utils/types.h>

namespace NVocal {

struct TServerConfig {
    ui16 Port;
    size_t MaxConnections;
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
