#include <QDebug>

#include <projects/vocal/vocal_lib/compress.h>

#include "network.h"

TNetwork::TNetwork() {
}

void TNetwork::ConnectToServer(QHostAddress address, quint16 port) {
    Socket.connectToHost(address, port);
    connect(&Socket, &QUdpSocket::readyRead, this, &TNetwork::OnDataReceived);
}

void TNetwork::SendControl(Space::TControl control) {
    if (!Socket.isOpen()) {
        return;
    }
    QByteArray data;
    data.resize(control.ByteSize());
    control.SerializeToArray(data.data(), data.size());
    Socket.write(data);
}

void TNetwork::OnDataReceived() {
    std::string data;
    data.resize(4096);
    data.resize(Socket.readDatagram(&data[0], data.size()));

    data = NVocal::Decompress(data);

    Space::TWorld world;
    if (world.ParseFromArray(data.data(), data.size())) {
        emit OnWorldReceived(world);
    } else {
        qDebug() << "failed to parse world";
    }
}
