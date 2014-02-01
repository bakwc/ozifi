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
        CheckSelection(SelectionFrom, event.pos());
    } else {
        MousePressed = false;
        CheckSelection(SelectionFrom, event.pos());
        World->Selection.reset();
    }
}

void TControl::OnMouseMove(QMouseEvent event) {
    if (MousePressed) {
        World->UpdateSelection(SelectionFrom, event.pos());
        CheckSelection(SelectionFrom, event.pos());
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

// check if to rects intersects
inline bool Intersects(int ax1, int ax2, int ay1, int ay2, int bx1, int bx2, int by1, int by2) {
    if (ax1 > ax2) std::swap(ax1, ax2);
    if (ay1 > ay2) std::swap(ay1, ay2);
    if (bx1 > bx2) std::swap(bx1, bx2);
    if (by1 > by2) std::swap(by1, by2);
    if (((ax1 >= bx1 && ax1 <= bx2) ||
         (ax2 >= bx1 && ax2 <= bx2) ||
         (bx1 >= ax1 && bx1 <= ax2) ||
         (bx2 >= ax1 && bx2 <= ax2)) &&
        ((ay1 >= by1 && ay1 <= by2) ||
         (ay2 >= by1 && ay2 <= by2) ||
         (by1 >= ay1 && by1 <= ay2) ||
         (by2 >= ay1 && by2 <= ay2)))
    {
        return true;
    }
    return false;
}

void TControl::CheckSelection(QPoint from, QPoint to) {
    World->SelectedPlanets.clear();
    for (size_t i = 0; i < World->planets_size(); ++i) {
        int ax1 = from.x();
        int ay1 = from.y();
        int ax2 = to.x();
        int ay2 = to.y();
        float r = World->planets(i).radius() * World->Scale;
        int bx1 = World->planets(i).x() * World->Scale + World->OffsetX - r;
        int bx2 = World->planets(i).x() * World->Scale + World->OffsetX + r;
        int by1 = World->planets(i).y() * World->Scale + World->OffsetY - r;
        int by2 = World->planets(i).y() * World->Scale + World->OffsetY + r;
        if (Intersects(ax1, ax2, ay1, ay2, bx1, bx2, by1, by2)) {
            World->SelectedPlanets.insert(World->planets(i).id());
        }
    }
}
