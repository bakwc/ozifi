#include "control.h"

TControl::TControl(TWorld *world)
    : World(world)
{
    LastSendControl.restart();
    startTimer(300);
}

void TControl::OnMouseEvent(QMouseEvent event, bool pressed) {
}

void TControl::OnResizeEvent(QResizeEvent event) {
}

void TControl::timerEvent(QTimerEvent *) {
    if (LastSendControl.elapsed() < 300) {
        return;
    }
    Space::TControl control;
    control.set_planetfrom(-1);
    control.set_planetto(-1);
    control.set_playername(World->PlayerName.toStdString());
    control.set_energypercent(0);
    emit OnControl(control);
    LastSendControl.restart();
}
