#pragma once

#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QTcpServer>
#include <QHash>
#include <QTime>

#include <utils/types.h>

#include <projects/space_capture/lib/space.h>

struct TClient {
    size_t Id;
    QHostAddress Address;
    quint16 Port;
    QTime LastActivity;
    QTcpSocket* Socket;
    std::string Buffer;
};

const size_t CLIENT_TIMEOUT = 5000;

class TNetwork: public QObject
{
    Q_OBJECT
public:
    TNetwork(ui16 port);
    virtual ~TNetwork();
signals:
    void OnControlReceived(size_t playerId, const std::string& command);
    void OnNewPlayerConnected(size_t playerId);
    void OnPlayerDisconnected(size_t playerId);
public slots:
    void OnClientConnected();
    void SendWorld(uint8_t playerId, const std::string& data);
    void SendCommand(const std::string& command);
private:
    QTcpServer Socket;
    QHash<QString, TClient> Clients;
    QHash<size_t, TClient*> ClientsById;
};
