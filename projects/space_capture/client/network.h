#pragma once

#include <QtNetwork/QUdpSocket>

#include <projects/space_capture/lib/space.h>

class TNetwork: public QObject {
    Q_OBJECT
public:
    explicit TNetwork();
    void ConnectToServer(QHostAddress address, quint16 port);
public slots:
    void SendControl(NSpace::TAttackCommand control);
private slots:
    void OnDataReceived();
signals:
    void OnWorldReceived(NSpace::TWorld world);
private:
    QUdpSocket Socket;
};
