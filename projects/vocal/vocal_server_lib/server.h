#pragma once

#include <string>
#include <memory>
#include <iostream>
#include <utils/types.h>

namespace NVocal {

struct TServerConfig {
    ui16 Port;                  // local port
    ui16 AdminPort;             // web admin port
    std::string DataDirectory;  // directory for database
};

class TServerImpl;
class TServer {
public:
    TServer(const TServerConfig& config);
    ~TServer();
    // total status
    void PrintStatus(std::ostream& out);
    // status for given client login
    void PrintClientStatus(const std::string& client, std::ostream& out);
private:
    std::unique_ptr<TServerImpl> Impl;
};

}
