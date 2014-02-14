#include "application.h"

TApplication::TApplication(int& argc, char **argv)
    : QApplication(argc, argv)
    , Control(&World)
    , WorldDisplay(&World)
    , State(AS_None)
{
    QObject::connect(&Control, &TControl::OnControl, &Network, &TNetwork::SendControl);
    QObject::connect(&Network, &TNetwork::OnWorldReceived, &World, &TWorld::UpdateWorld);
    QObject::connect(&World, &TWorld::OnWorldUpdated, &Display, &TDisplay::Render);
}

void TApplication::Init() {
    Display.UpdateContol(&Control);
    Display.UpdateDisplay(&WorldDisplay);
    Network.ConnectToServer(QHostAddress("127.0.0.1"), 9999);

//    Display.UpdateContol(&MainMenu);
//    Display.UpdateDisplay(&MainMenu);

    Display.show();
}
