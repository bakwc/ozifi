#include <ctime>

#include <QCoreApplication>

#include "network.h"
#include "world.h"

int main(int argc, char *argv[])
{
    srand(time(NULL));
    QCoreApplication a(argc, argv);
    TNetwork network;
    TWorld world;
    QObject::connect(&network, &TNetwork::OnControlReceived, &world, &TWorld::OnControl);
    QObject::connect(&network, &TNetwork::OnNewPlayerConnected, &world, &TWorld::OnNewPlayer);
    QObject::connect(&network, &TNetwork::OnPlayerDisconnected, &world, &TWorld::OnPlayerLeft);
    QObject::connect(&world, &TWorld::SendWorldToPlayer, &network, &TNetwork::SendWorld);
    return a.exec();
}
