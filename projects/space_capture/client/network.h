#pragma once

#include <QtNetwork/QUdpSocket>

#include <projects/space_capture/lib/space.pb.h>

class TNetwork: public QObject {
    Q_OBJECT
public:
    explicit TNetwork();
    void ConnectToServer(QHostAddress address, quint16 port);
public slots:
    void SendControl(Space::TControl control);
private slots:
    void OnDataReceived();
signals:
    void OnWorldReceived(Space::TWorld world);
private:
    QUdpSocket Socket;
};
