#include <netdb.h>
#include <stdio.h>
#include <cstring>
#include <sstream>

#include <netinet/in.h>
#include <fcntl.h>

#include "network.h"

using namespace std;

TNetwork::TNetwork(const std::string& address, uint16_t port, TOnWorldUpdate onUpdate)
    : OnWorldReceived(onUpdate)
    , Finished(false)
{
    Socket = socket(AF_INET, SOCK_DGRAM, 0);
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

void TNetwork::SendControl(NSpace::TAttackCommand control) {
    std::stringstream out;
    ::Save(out, control);
    std::string data = out.str();
    send(Socket, data.c_str(), data.size(), 0);
}

void TNetwork::Worker() {
    int flags = fcntl(Socket, F_GETFL, 0);
    fcntl(Socket, F_SETFL, flags | O_NONBLOCK);
    std::string buff;
    buff.resize(4096);
    while (!Finished) {
        ssize_t ret = recv(Socket, &buff[0], buff.size(), 0);
        if (ret > 0) {
            NSpace::TWorld world;
            imemstream in(&buff[0], ret);
            try {
                ::Load(in, world);
                OnWorldReceived(world);
            } catch(...) {
            }
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}
