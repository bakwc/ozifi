#include <QDebug>
#include "network.h"

TNetwork::TNetwork() {
}

void TNetwork::ConnectToServer(QHostAddress address, quint16 port) {
    Socket.connectToHost(address, port);
    connect(&Socket, &QUdpSocket::readyRead, this, &TNetwork::OnDataReceived);
}

void TNetwork::SendControl(Space::TControl control) {
    QByteArray data;
    data.resize(control.ByteSize());
    control.SerializeToArray(data.data(), data.size());
    Socket.write(data);
}

void TNetwork::OnDataReceived() {
    QByteArray data = Socket.readAll();
    Space::TWorld world;
    if (world.ParseFromArray(data.data(), data.size())) {
        emit OnWorldReceived(world);
    } else {
        qDebug() << "failed to parse world";
    }
}
