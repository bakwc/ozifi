#include <QDebug>

#include "world.h"

TWorld::TWorld()
    : Scale(1.0)
    , OffsetX(0)
    , OffsetY(0)
    , Power(50)
{
}

void TWorld::UpdateWorld(Space::TWorld world) {
    if (world.planets_size() != 0) {
        Space::TWorld::operator =(world);
    } else {
        *mutable_ships() = world.ships();
    }
    IdToPlayer.clear();
    for (size_t i = 0; i < players_size(); ++i) {
        IdToPlayer[players(i).id()] = mutable_players(i);
    }
    emit OnWorldUpdated();
}

void TWorld::UpdateSelection(QPointF from, QPointF to) {
    Selection = {from, to};
}

void TWorld::RemoveSelection() {
    Selection.reset();
}

Space::TPlayer *TWorld::SelfPlayer() {
    if (IdToPlayer.find(this->selfid()) == IdToPlayer.end()) {
        qDebug() << "SelfPlayer(): player with id" << this->selfid() << "missing";
        return nullptr;
    }
    return IdToPlayer[this->selfid()];
}
