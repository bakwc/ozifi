#include <QDebug>

#include "network.h"

TNetwork::TNetwork()
    : QObject(NULL)
    , CurrentId(0)
{
    Socket.bind(9999);
    QObject::connect(&Socket, &QUdpSocket::readyRead, this, &TNetwork::onDataReceived);
}

TNetwork::~TNetwork() {
}

void TNetwork::onDataReceived() {
    QByteArray buff;
    buff.resize(512);
    QHostAddress fromAddr;
    quint16 fromPort;
    qint64 received = Socket.readDatagram(buff.data(), buff.size(), &fromAddr, &fromPort);
    buff.resize(received);
    QString senderAddr = fromAddr.toString() + ":" + QString("%1").arg(fromPort);
    Space::TControl control;
    if (!control.ParseFromArray(buff.data(), buff.size())) {
        qDebug() << "failed to parse packet from client " << senderAddr;
    }
    auto it = Clients.find(senderAddr);
    if (it == Clients.end()) {
        TClient client;
        client.Id = CurrentId;
        client.Address = fromAddr;
        client.Port = fromPort;
        Clients[senderAddr] = client;
        qDebug() << "Player connected: " << senderAddr;
        emit onNewPlayerConnected(CurrentId);
        ++CurrentId;
    }
    TClient& client = Clients[senderAddr];
    emit onControlReceived(client.Id, control);
}

void TNetwork::SendWorld(Space::TWorld world) {
    QByteArray data;
    data.resize(world.ByteSize());
    world.SerializeToArray(data.data(), data.size());
    for (auto& client: Clients) {
        Socket.writeDatagram(data, client.Address, client.Port);
    }
}
