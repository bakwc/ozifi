#include <netdb.h>
#include <stdio.h>
#include <cstring>
#include <sstream>

#include <netinet/in.h>
#include <fcntl.h>

#include "network.h"

#include "../../lib/defines.h"

using namespace std;

TNetwork::TNetwork(TOnWorldUpdate onUpdate,
                   TOnCommandReceived onCommand)
    : OnWorldReceived(onUpdate)
    , OnCommand(onCommand)
    , Finished(false)
{

    Socket = socket(AF_INET, SOCK_STREAM, 0);
    Socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    struct hostent *he;
    if(Socket < 0) {
        perror("socket");
        throw 43;
    }
    if ((he = gethostbyname(CONTROL_SERVER_ADDRESS)) == NULL ) {
         perror("socket");
         throw 43;
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons(CONTROL_SERVER_PORT);
    memcpy(&addr.sin_addr, he->h_addr_list[0], he->h_length);
    connect(Socket, (struct sockaddr *)&addr, sizeof(addr));
    std::string request = "GET /quick HTTP/1.0\r\n\r\n";
    send(Socket, request.data(), request.size(), 0);
    std::string buff;
    buff.resize(2048);
    ssize_t ret = recv(Socket, &buff[0], buff.size(), 0);
    if (ret < 0) {
        std::cerr << "Error!\n";
        throw 43;
    }
    buff.resize(ret);
    size_t n = buff.find("\r\n\r\n");
    if (n == std::string::npos) {
        std::cerr << "failed to connect\n";
        throw 42;
    }
    buff = buff.substr(n + 4);
    n = buff.find(":");
    if (n == std::string::npos) {
        std::cerr << "failed to connect\n";
        throw 42;
    }
    std::string host = buff.substr(0, n);
    std::string portStr = buff.substr(n + 1);
    std::istringstream ss(portStr);
    uint16_t gamePort;
    ss >> gamePort;
    shutdown(Socket, SHUT_RDWR);


    Socket = socket(AF_INET, SOCK_STREAM, 0);
    if(Socket < 0) {
        perror("socket");
        throw 43;
    }
    if ((he = gethostbyname(host.c_str())) == NULL ) {
         perror("socket");
         throw 43;
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons(gamePort);
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
