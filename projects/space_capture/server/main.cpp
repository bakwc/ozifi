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
    QObject::connect(&network, &TNetwork::onControlReceived, &world, &TWorld::OnControl);
    QObject::connect(&network, &TNetwork::onNewPlayerConnected, &world, &TWorld::OnNewPlayer);
    QObject::connect(&world, &TWorld::WorldUpdated, &network, &TNetwork::SendWorld);
    return a.exec();
}
