#pragma once

#include <QtNetwork/QUdpSocket>
#include <QHash>

#include <projects/space_capture/lib/space.pb.h>

struct TClient {
    size_t Id;
    QHostAddress Address;
    quint16 Port;
};

class TNetwork: public QObject
{
    Q_OBJECT
public:
    TNetwork();
    virtual ~TNetwork();
signals:
    void onControlReceived(size_t playerId, Space::TControl control);
    void onNewPlayerConnected(size_t playerId);
public slots:
    void onDataReceived();
    void SendWorld(Space::TWorld world, size_t playerId);
private:
    QUdpSocket Socket;
    QHash<QString, TClient> Clients;
    QHash<size_t, TClient*> ClientsById;
    size_t CurrentId;
};
