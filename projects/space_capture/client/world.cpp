#include <QDebug>

#include "world.h"

TWorld::TWorld()
    : ScaleX(1.0)
    , ScaleY(1.0)
{
}

void TWorld::UpdateWorld(Space::TWorld world) {
    Space::TWorld::operator =(world);
    IdToPlayer.clear();
    for (size_t i = 0; i < players_size(); ++i) {
        IdToPlayer[players(i).id()] = mutable_players(i);
    }
    emit OnWorldUpdated();
}
