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

#include "application.h"

int main(int argc, char *argv[]) {
    TApplication application(argc, argv);
    application.Init();
    return application.exec();
}
