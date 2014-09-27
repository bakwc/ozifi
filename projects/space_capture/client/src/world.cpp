#include "world.h"

void TWorld::UpdateWorld(const NSpace::TWorld& world) {
    if (world.Planets.size() != 0 || world.RoundStartsAt != uint8_t(-1) || world.WaitingPlayers) {
        NSpace::TWorld::operator =(world);
    } else {
        Ships = world.Ships;
    }
    IdToPlayer.clear();
    for (size_t i = 0; i < Players.size(); ++i) {
        IdToPlayer[Players[i].ID] = &Players[i];
    }
    if (OnWorldUpdated) {
        OnWorldUpdated();
    }
}

void TWorld::UpdateSelection(TPoint from, TPoint to) {
    Selection = {from, to};
    HaveSelection = true;
}

void TWorld::RemoveSelection() {
    HaveSelection = false;
}

NSpace::TPlayer* TWorld::SelfPlayer() {
    if (IdToPlayer.find(this->SelfId) == IdToPlayer.end()) {
        std::cerr << "SelfPlayer(): player with id " << this->SelfId << " missing" << "\n";
        return nullptr;
    }
    return IdToPlayer[this->SelfId];
}
