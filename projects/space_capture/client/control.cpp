#include "control.h"

TControl::TControl(TWorld *world)
    : World(world)
{
    LastSendControl.restart();
    startTimer(300);
}

void TControl::OnMouseEvent(QMouseEvent event, bool pressed) {
    if (pressed) {
        SelectionFrom = event.pos();
        MousePressed = true;
    } else {
        MousePressed = false;
        World->Selection.reset();
        //CheckSelection(SelectionFrom, event.pos());
    }
}

void TControl::OnMouseMove(QMouseEvent event) {
    if (MousePressed) {
        World->UpdateSelection(SelectionFrom, event.pos());
    }
}

void TControl::OnResizeEvent(QResizeEvent event) {
    int newWidth = event.size().width();
    int newHeight = event.size().height();
    int realWidth, realHeight;
    float rate = WORLD_HEIGHT / WORLD_WIDTH;
    if (newWidth * rate < newHeight) {
        realWidth = newWidth;
        realHeight = newWidth * rate;
    } else {
        realHeight = newHeight;
        realWidth = newHeight / rate;
    }
    World->Scale = realWidth / WORLD_WIDTH;
    World->OffsetX = (newWidth - realWidth) / 2;
    World->OffsetY = (newHeight - realHeight) / 2;
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
