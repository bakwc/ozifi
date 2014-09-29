#pragma once

#include <QObject>
#include <memory>

#include "network.h"

#include "../lib/world.h"

//#include "world.h"

class TGameServer : public QObject
{
    Q_OBJECT
public:
    explicit TGameServer(ui16 port, QObject *parent = 0);
    size_t GetPlayersNumber() const;
    bool Empty() const;
    bool Full() const;
private:
    void timerEvent(QTimerEvent*);
private:
    TNetwork Network;
    std::unique_ptr<NSpaceEngine::TWorld> World;
//    TWorld World;
};
