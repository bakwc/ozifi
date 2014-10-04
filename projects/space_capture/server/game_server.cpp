#include "game_server.h"

TGameServer::TGameServer(ui16 port, QObject *parent)
    : QObject(parent)
    , Network(port)
{
    World.reset(new NSpaceEngine::TWorld(true, [this](const std::string& command) {
        Network.SendCommand(command);
    }));
    World->Generator.seed(rand() % 4096);
    connect(&Network, &TNetwork::OnControlReceived,
            [this](size_t playerId, const std::string& command)
    {
        World->PlayCommand(command);
    });
    connect(&Network, &TNetwork::OnNewPlayerConnected, [this] (size_t playerId) {
        Network.SendWorld(playerId, World->Serialize());
        World->OnNewPlayer(playerId);
    });
    connect(&Network, &TNetwork::OnPlayerDisconnected, [this] (size_t playerId) {
        World->OnPlayerLeft(playerId);
    });
    startTimer(50);
}

size_t TGameServer::GetPlayersNumber() const {
    return World->GetPlayersCount();
}

bool TGameServer::Empty() const {
    return World->Empty();
}

bool TGameServer::Full() const {
    return World->Full();
}

void TGameServer::timerEvent(QTimerEvent*) {
    World->Process();
}
