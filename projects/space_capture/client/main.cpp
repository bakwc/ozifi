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

// scroll кнопками
// 80 в начале игры
// регистрация, рейтинг, чат
// выеделение планет на расстоянии
// запретить вылет кораблей из собственной планеты
// сделать чтоб корабли, улетевшие вперёд, не возвращались
// сделать возможность изменять цель кораблей
