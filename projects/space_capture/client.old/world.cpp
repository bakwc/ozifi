#include <QDebug>

#include "world.h"

TWorld::TWorld()
    : Scale(1.0)
    , OffsetX(0)
    , OffsetY(0)
    , Power(50)
{
}

void TWorld::UpdateWorld(NSpace::TWorld world) {
    if (world.Planets.size() != 0 || world.RoundStartsAt != uint8_t(-1) || world.WaitingPlayers) {
        NSpace::TWorld::operator =(world);
    } else {
        Ships = world.Ships;
    }
    IdToPlayer.clear();
    for (size_t i = 0; i < Players.size(); ++i) {
        IdToPlayer[Players[i].ID] = &Players[i];
    }
    emit OnWorldUpdated();
}

void TWorld::UpdateSelection(QPointF from, QPointF to) {
    Selection = {from, to};
}

void TWorld::RemoveSelection() {
    Selection.reset();
}

NSpace::TPlayer* TWorld::SelfPlayer() {
    if (IdToPlayer.find(this->SelfId) == IdToPlayer.end()) {
        qDebug() << "SelfPlayer(): player with id" << this->SelfId << "missing";
        return nullptr;
    }
    return IdToPlayer[this->SelfId];
}
