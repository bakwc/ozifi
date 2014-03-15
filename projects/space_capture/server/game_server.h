#pragma once

#include <QObject>

#include "network.h"
#include "world.h"

class TGameServer : public QObject
{
    Q_OBJECT
public:
    explicit TGameServer(ui16 port, QObject *parent = 0);
    size_t GetPlayersNumber() const;
    bool Empty() const;
    bool Full() const;
private:
    TNetwork Network;
    TWorld World;
};
