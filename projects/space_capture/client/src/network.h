#pragma once

#include <functional>
#include <string>
#include <thread>
#include <sys/socket.h>
#include "../../lib/space.h"

using TOnWorldUpdate = std::function<void(const NSpace::TWorld&)>;
class TNetwork {
public:
    explicit TNetwork(const std::string& address, uint16_t port, TOnWorldUpdate onUpdate);
    ~TNetwork();
    void SendControl(NSpace::TAttackCommand control);
private:
    void Worker();
private:
    TOnWorldUpdate OnWorldReceived;
    std::unique_ptr<std::thread> WorkerThread;
    bool Finished;
    int Socket;
};
