#pragma once

#include <QtNetwork/QUdpSocket>
#include <QHash>
#include <QTime>

#include <projects/space_capture/lib/space.pb.h>

struct TClient {
    size_t Id;
    QHostAddress Address;
    quint16 Port;
    QTime LastActivity;
};

const size_t CLIENT_TIMEOUT = 5000;

class TNetwork: public QObject
{
    Q_OBJECT
public:
    TNetwork();
    virtual ~TNetwork();
signals:
    void OnControlReceived(size_t playerId, Space::TControl control);
    void OnNewPlayerConnected(size_t playerId);
    void OnPlayerDisconnected(size_t playerId);
public slots:
    void OnDataReceived();
    void SendWorld(Space::TWorld world, size_t playerId);
private:
    void timerEvent(QTimerEvent*);
private:
    QUdpSocket Socket;
    QHash<QString, TClient> Clients;
    QHash<size_t, TClient*> ClientsById;
};
