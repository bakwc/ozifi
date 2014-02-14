#pragma once

#include <QApplication>

#include "world.h"
#include "control.h"
#include "display.h"
#include "world_display.h"
#include "main_menu.h"
#include "network.h"

enum EApplicationState {
    AS_None,
    AS_MainMenu,
    AS_Game
};

class TApplication : public QApplication
{
    Q_OBJECT
public:
    explicit TApplication(int& argc, char** argv);
    void Init();
signals:
public slots:
private:
    TWorld World;
    TControl Control;
    TWorldDisplay WorldDisplay;
    TDisplay Display;
    TMainMenu MainMenu;
    TNetwork Network;
    EApplicationState State;
};
