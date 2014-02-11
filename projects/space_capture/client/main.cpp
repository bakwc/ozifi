#include <QApplication>
#include <QMessageBox>

/*      MVC
 * display = view
 * world = model
 * control, network = controls */

#include "display.h"
#include "control.h"
#include "network.h"
#include "world.h"

int main(int argc, char *argv[])
{
    QString serverAddr = "127.0.0.1";
    if (argc >= 2) {
        serverAddr = argv[1];
    }

    qDebug() << serverAddr;

    QApplication application(argc, argv);
    TWorld world;
    TControl control(&world);

    QGLFormat f = QGLFormat::defaultFormat();
    f.setSampleBuffers(true);
    QGLFormat::setDefaultFormat(f);

    TDisplay display(&world);

    TNetwork network;

    network.ConnectToServer(QHostAddress(serverAddr), 9999);

    QObject::connect(&control, &TControl::OnControl, &network, &TNetwork::SendControl);
    QObject::connect(&network, &TNetwork::OnWorldReceived, &world, &TWorld::UpdateWorld);
    QObject::connect(&world, &TWorld::OnWorldUpdated, &display, &TDisplay::RedrawWorld);
    QObject::connect(&display, &TDisplay::OnMouseEvent, &control, &TControl::OnMouseEvent);
    QObject::connect(&display, &TDisplay::OnMouseMove, &control, &TControl::OnMouseMove);
    QObject::connect(&display, &TDisplay::OnResized, &control, &TControl::OnResizeEvent);
    QObject::connect(&display, &TDisplay::OnWheelEvent, &control, &TControl::OnWheelEvent);

    display.show();

    return application.exec();
}
