#include <sstream>

#include <QDebug>

#include <projects/vocal/vocal_lib/compress.h>

#include "network.h"

TNetwork::TNetwork() {
}

void TNetwork::ConnectToServer(QHostAddress address, quint16 port) {
    Socket.connectToHost(address, port);
    connect(&Socket, &QUdpSocket::readyRead, this, &TNetwork::OnDataReceived);
}

void TNetwork::SendControl(NSpace::TAttackCommand control) {
    if (!Socket.isOpen()) {
        return;
    }
    std::stringstream out;
    ::Save(out, control);
    std::string data = out.str();
    Socket.write(&data[0], data.size());
}

void TNetwork::OnDataReceived() {
    std::string data;
    data.resize(4096);
    data.resize(Socket.readDatagram(&data[0], data.size()));
    imemstream in(&data[0], data.size());

    NSpace::TWorld world;
    ::Load(in, world);
    emit OnWorldReceived(world);
}
