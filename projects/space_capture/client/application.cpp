#include "application.h"

TApplication::TApplication(int& argc, char **argv)
    : QApplication(argc, argv)
    , Control(&World)
    , WorldDisplay(&World)
    , State(AS_None)
{
    connect(&Control, &TControl::OnControl, &Network, &TNetwork::SendControl);
    connect(&Network, &TNetwork::OnWorldReceived, &World, &TWorld::UpdateWorld);
    connect(&World, &TWorld::OnWorldUpdated, &Display, &TDisplay::Render);
    connect(&MainMenu, &TMainMenu::QuickGame, this, &TApplication::QuickGame);
    connect(&MainMenu, &TMainMenu::Exit, this, &TApplication::Exit);
}

void TApplication::Init() {
    Display.UpdateContol(&MainMenu);
    Display.UpdateDisplay(&MainMenu);
    connect(&MainMenu, &TMainMenu::Render, &Display, &TDisplay::Render);

    Display.show();
}

void TApplication::QuickGame() {
    disconnect(&MainMenu, &TMainMenu::Render, &Display, &TDisplay::Render);
    Display.UpdateContol(&Control);
    Display.UpdateDisplay(&WorldDisplay);
    Network.ConnectToServer(QHostAddress("127.0.0.1"), 9999);
}

void TApplication::Exit() {
    exit(0);
}
