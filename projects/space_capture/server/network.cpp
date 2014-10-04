#include <sstream>

#include <QDebug>

#include <projects/vocal/vocal_lib/compress.h>
#include <projects/space_capture/lib/defines.h>

#include "network.h"

TNetwork::TNetwork(ui16 port)
    : QObject(NULL)
{
    Socket.listen(QHostAddress::Any, port);
    QObject::connect(&Socket, &QTcpServer::newConnection, this, &TNetwork::OnClientConnected);
}

TNetwork::~TNetwork() {
}

void TNetwork::OnClientConnected() {
    if (Clients.size() == MAX_PLAYERS) {
        qDebug() << "[WARNING] server is full";
        return;
    }
    int id = rand() % MAX_PLAYERS;
    while (ClientsById.find(id) != ClientsById.end()) {
        id = rand() % MAX_PLAYERS;
    }

    TClient client;
    client.Socket = Socket.nextPendingConnection();
    client.Id = id;
    client.Address = client.Socket->peerAddress();
    client.Port = client.Socket->peerPort();
    client.LastActivity.start();
    QString senderAddr = client.Address.toString() + ":" + QString("%1").arg(client.Port);
    Clients[senderAddr] = client;
    ClientsById[client.Id] = &Clients[senderAddr];
    qDebug() << "[INFO] Player connected: " << senderAddr;

    QTcpSocket* socket = client.Socket;
    socket->setSocketOption(QTcpSocket::LowDelayOption, 1);

    QObject::connect(client.Socket, &QTcpSocket::readyRead, [this, socket, senderAddr] {
        QByteArray data = socket->readAll();
        auto it = Clients.find(senderAddr);
        TClient& client = Clients[senderAddr];
        client.Buffer += std::string(data.data(), data.size());
        client.LastActivity.restart();
        while (true) {
            std::string command;
            try {
                imemstream in(client.Buffer.data(), client.Buffer.size());
                ::Load(in, command);
                if (!in) {
                    throw 0;
                }
                emit OnControlReceived(client.Id, command);
                client.Buffer = client.Buffer.substr(in.pos());
            } catch (...) {
                break;
            }
        }
    });
    QObject::connect(client.Socket, &QTcpSocket::disconnected, [this, senderAddr] {
        auto it = Clients.find(senderAddr);
        emit OnPlayerDisconnected(it.value().Id);
        ClientsById.remove(it.value().Id);
        Clients.erase(it);
    });

    emit OnNewPlayerConnected(id);
}

void TNetwork::SendCommand(const std::string& command) {

    std::stringstream out;
    ::Save(out, command);
    std::string packet = out.str();

    for (auto&& cli: Clients) {
        cli.Socket->write(packet.data(), packet.size());
    }
}

void TNetwork::SendWorld(uint8_t playerId, const std::string& data) {
    auto cliIt = ClientsById.find(playerId);
    if (cliIt == ClientsById.end()) {
        return;
    }
    TClient* client = cliIt.value();

    std::stringstream out;
    ::SaveMany(out, playerId, data);
    std::string packet = out.str();

    client->Socket->write(packet.data(), packet.size());
}
