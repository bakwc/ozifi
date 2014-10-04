#pragma once

#include <functional>
#include <string>
#include <thread>
#include <sys/socket.h>
#include "../../lib/space.h"

using TOnWorldUpdate = std::function<void(size_t, const std::string&)>;
using TOnCommandReceived = std::function<void(const std::string&)>;
class TNetwork {
public:
    explicit TNetwork(TOnWorldUpdate onUpdate,
                      TOnCommandReceived onCommand);
    ~TNetwork();
    void SendCommand(const std::string& command);
private:
    void Worker();
private:
    TOnWorldUpdate OnWorldReceived;
    TOnCommandReceived OnCommand;
    std::unique_ptr<std::thread> WorkerThread;
    bool Finished;
    bool HasWorld = false;
    int Socket;
};
