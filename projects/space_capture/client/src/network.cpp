#include <netdb.h>
#include <stdio.h>
#include <cstring>
#include <sstream>

#include <netinet/in.h>
#include <fcntl.h>

#include "network.h"

using namespace std;

TNetwork::TNetwork(const std::string& address, uint16_t port,
                   TOnWorldUpdate onUpdate,
                   TOnCommandReceived onCommand)
    : OnWorldReceived(onUpdate)
    , OnCommand(onCommand)
    , Finished(false)
{
    Socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    struct hostent *he;
    if(Socket < 0)
    {
        perror("socket");
        throw 43;
    }
    if ((he = gethostbyname(address.c_str())) == NULL ) {
         perror("socket");
         throw 43;
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    memcpy(&addr.sin_addr, he->h_addr_list[0], he->h_length);
    connect(Socket, (struct sockaddr *)&addr, sizeof(addr));
    WorkerThread.reset(new std::thread([this] {
        Worker();
    }));
}

TNetwork::~TNetwork() {
    shutdown(Socket, SHUT_RDWR);
    Finished = true;
    WorkerThread->join();
}

void TNetwork::SendCommand(const std::string& command) {
    std::stringstream out;
    ::Save(out, command);
    std::string packet = out.str();

    send(Socket, packet.c_str(), packet.size(), 0);
}


void TNetwork::Worker() {
    int flags = fcntl(Socket, F_GETFL, 0);
    fcntl(Socket, F_SETFL, flags | O_NONBLOCK);
    std::string totalBuff;
    std::string buff;
    buff.resize(4096);
    while (!Finished) {
        ssize_t ret = recv(Socket, &buff[0], buff.size(), 0);
        if (ret > 0) {
            totalBuff += std::string(&buff[0], ret);
            while (true) {
                imemstream in(&totalBuff[0], totalBuff.size());
                if (!HasWorld) {
                    uint8_t selfId;
                    std::string world;
                    try {
                        ::LoadMany(in, selfId, world);
                        if (!in) {
                            throw 0;
                        }
                    } catch(...) {
                        break;
                    }

                    OnWorldReceived(selfId, world);
                    HasWorld = true;
                } else {
                    std::string command;
                    try {
                        ::Load(in, command);
                        if (!in) {
                            throw 0;
                        }
                    } catch(...) {
                        break;
                    }
                    OnCommand(command);
                }
                totalBuff = totalBuff.substr(in.pos());
            }
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}
