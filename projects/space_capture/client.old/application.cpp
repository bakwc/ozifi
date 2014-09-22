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
    try {
        optional<std::string> address;
        address = NHttpFetcher::FetchUrl("http://" + std::string(CONTROL_SERVER_ADDRESS) +
                                         ":" + ToString(CONTROL_SERVER_PORT) + "/quick");

        if (!address.is_initialized()) {
            throw UException("Failed to get servers list");
        }

        QString hostPortStr = QString::fromStdString(*address);
        QStringList hostPort = hostPortStr.split(':');
        if (!hostPort.size() == 2) {
            throw UException("Failed to get servers list");
        }
        ui16 port = FromString(hostPort[1].toStdString());

        disconnect(&MainMenu, &TMainMenu::Render, &Display, &TDisplay::Render);
        Display.UpdateContol(&Control);
        Display.UpdateDisplay(&WorldDisplay);
        Network.ConnectToServer(QHostAddress(hostPort[0]), port);
    } catch (const std::exception& e) {
        qDebug() << "Failed to start quick game:" << e.what();
    }


}

void TApplication::Exit() {
    exit(0);
}
