#include <utils/cast.h>
#include <library/http_fetcher/fetcher.h>

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

    optional<std::string> address;
    address = NHttpFetcher::FetchUrl("http://" + std::string(CONTROL_SERVER_ADDRESS) +
                                     ":" + ToString(CONTROL_SERVER_PORT));

    if (!address.is_initialized()) {
        qDebug() << "Failed to get servers list";
        return;
    }

    QString hostPortStr = QString::fromStdString(*address);
    QStringList hostPort = hostPortStr.split(':');
    if (!hostPort.size() == 2) {
        qDebug() << "Wrong address:" << hostPortStr;
        return;
    }

    Network.ConnectToServer(QHostAddress(hostPort[0]), FromString(hostPort[1].toStdString()));
}

void TApplication::Exit() {
    exit(0);
}
