#include "game_server.h"

TGameServer::TGameServer(ui16 port, QObject *parent)
    : QObject(parent)
    , Network(port)
{
    connect(&Network, &TNetwork::OnControlReceived, &World, &TWorld::OnControl);
    connect(&Network, &TNetwork::OnNewPlayerConnected, &World, &TWorld::OnNewPlayer);
    connect(&Network, &TNetwork::OnPlayerDisconnected, &World, &TWorld::OnPlayerLeft);
    connect(&World, &TWorld::SendWorldToPlayer, &Network, &TNetwork::SendWorld);
}

size_t TGameServer::GetPlayersNumber() const {
    return World.GetPlayersCount();
}

bool TGameServer::Empty() const {
    return World.Empty();
}

bool TGameServer::Full() const {
    return World.Full();
}
