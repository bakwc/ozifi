#include <sstream>

#include <QDebug>

#include <projects/vocal/vocal_lib/compress.h>
#include <projects/space_capture/lib/defines.h>

#include "network.h"

TNetwork::TNetwork(ui16 port)
    : QObject(NULL)
{
    Socket.bind(port);
    QObject::connect(&Socket, &QUdpSocket::readyRead, this, &TNetwork::OnDataReceived);
    startTimer(100);
}

TNetwork::~TNetwork() {
}

void TNetwork::OnDataReceived() {
    QByteArray buff;
    buff.resize(512);
    QHostAddress fromAddr;
    quint16 fromPort;
    qint64 received = Socket.readDatagram(buff.data(), buff.size(), &fromAddr, &fromPort);
    buff.resize(received);
    QString senderAddr = fromAddr.toString() + ":" + QString("%1").arg(fromPort);
    NSpace::TAttackCommand command;
    imemstream in(buff.data(), buff.size());
    ::Load(in, command);
    auto it = Clients.find(senderAddr);
    if (it == Clients.end()) {
        if (Clients.size() == MAX_PLAYERS) {
            qDebug() << "server is full";
            return;
        }
        int id = rand() % MAX_PLAYERS;
        while (ClientsById.find(id) != ClientsById.end()) {
            id = rand() % MAX_PLAYERS;
        }

        TClient client;
        client.Id = id;
        client.Address = fromAddr;
        client.Port = fromPort;
        client.LastActivity.start();
        Clients[senderAddr] = client;
        ClientsById[client.Id] = &Clients[senderAddr];
        qDebug() << "Player connected: " << senderAddr;
        emit OnNewPlayerConnected(id);
    }
    TClient& client = Clients[senderAddr];
    client.LastActivity.restart();
    emit OnControlReceived(client.Id, command);
}

void TNetwork::SendWorld(NSpace::TWorld world, size_t playerId) {
    auto cliIt = ClientsById.find(playerId);
    if (cliIt == ClientsById.end()) {
        return;
    }
    TClient* client = cliIt.value();

    std::stringstream out;
    ::Save(out, world);
    std::string data = out.str();

    // todo: compress data

    Socket.writeDatagram(data.data(), data.size(), client->Address, client->Port);
}

void TNetwork::timerEvent(QTimerEvent*) {
    for (QHash<QString, TClient>::iterator it = Clients.begin(); it != Clients.end();) {
        if (it.value().LastActivity.elapsed() >= CLIENT_TIMEOUT) {
            emit OnPlayerDisconnected(it.value().Id);
            ClientsById.remove(it.value().Id);
            it = Clients.erase(it);
        } else {
            ++it;
        }
    }
}
