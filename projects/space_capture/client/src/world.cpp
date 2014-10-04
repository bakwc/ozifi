#include "world.h"

TWorld::TWorld(std::function<void(const std::string& command)> onCommand)
    : NSpaceEngine::TWorld(false, onCommand)
{
}

void TWorld::UpdateWorld(const std::string& data) {
    this->Deserialize(data);
}

void TWorld::OnCommandReceived(const std::string& command) {
    try {
        this->PlayCommand(command);
    } catch(const std::string& e) {
        std::cerr << "error: " << e << "\n";
        throw;
    }
}

void TWorld::UpdateSelection(TPoint from, TPoint to) {
    Selection = {from, to};
    HaveSelection = true;
}

void TWorld::RemoveSelection() {
    HaveSelection = false;
}

NSpaceEngine::TPlayer* TWorld::SelfPlayer() {
    if (Players.find(this->SelfId) == Players.end()) {
        std::cerr << "SelfPlayer(): player with id " << this->SelfId << " missing" << "\n";
        return nullptr;
    }
    return &Players[this->SelfId];
}

void TWorld::SetSeed(uint32_t seed) {
    Generator.seed(seed);
}
