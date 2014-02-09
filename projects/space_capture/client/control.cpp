#include "control.h"

TControl::TControl(TWorld *world)
    : World(world)
    , State(CS_None)
{
    LastSendControl.restart();
    startTimer(300);
}

void TControl::OnMouseEvent(QMouseEvent event, bool mouseDown) {
    if (State == CS_None) {
        CheckSelection(event.pos(), event.pos());
        if (!World->SelectedPlanets.empty()) {
            State = CS_TargetSelection;
        } else if (mouseDown) {
            State = CS_PlanetSelection;
            SelectionFrom = event.pos();
        }
    } else if (State == CS_PlanetSelection && !mouseDown) {
        if (!World->SelectedPlanets.empty()) {
            State = CS_TargetSelection;
            World->Selection.reset();
        } else {
            State = CS_None;
            World->SelectedPlanets.clear();
            World->Selection.reset();
            World->SelectedTarget.reset();
        }
    } else if (State == CS_TargetSelection && !mouseDown) {
        CheckTargetSelection(event.pos());
        if (World->SelectedTarget.is_initialized()) {
            if (World->SelectedPlanets.size() == 1 &&
                    *World->SelectedPlanets.begin() == *World->SelectedTarget)
            {
                CheckSelection(event.pos(), event.pos());
            } else {
                SpawnShips();
                State = CS_None;
                World->SelectedPlanets.clear();
                World->Selection.reset();
                World->SelectedTarget.reset();
            }
        } else {
            State = CS_None;
            World->SelectedPlanets.clear();
            World->Selection.reset();
            World->SelectedTarget.reset();
        }
    }
}

void TControl::OnMouseMove(QMouseEvent event) {
    if (State == CS_PlanetSelection) {
        World->UpdateSelection(SelectionFrom, event.pos());
        CheckSelection(SelectionFrom, event.pos());
    } else if (State == CS_TargetSelection) {
        CheckTargetSelection(event.pos());
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
    if (LastSendControl.elapsed() < 1000) {
        return;
    }
    Space::TControl control;
    emit OnControl(control);
    LastSendControl.restart();
}

// check if two rects intersects
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
    int ax1 = from.x();
    int ay1 = from.y();
    int ax2 = to.x();
    int ay2 = to.y();
    for (size_t i = 0; i < World->planets_size(); ++i) {
        if (World->planets(i).playerid() != World->selfid()) {
            continue;
        }
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

void TControl::CheckTargetSelection(QPoint position) {
    int ax1 = position.x();
    int ax2 = position.x();
    int ay1 = position.y();
    int ay2 = position.y();
    for (size_t i = 0; i < World->planets_size(); ++i) {
        float r = World->planets(i).radius() * World->Scale;
        int bx1 = World->planets(i).x() * World->Scale + World->OffsetX - r;
        int bx2 = World->planets(i).x() * World->Scale + World->OffsetX + r;
        int by1 = World->planets(i).y() * World->Scale + World->OffsetY - r;
        int by2 = World->planets(i).y() * World->Scale + World->OffsetY + r;
        if (Intersects(ax1, ax2, ay1, ay2, bx1, bx2, by1, by2)) {
            World->SelectedTarget = i;
            return;
        }
    }
    World->SelectedTarget.reset();
}

void TControl::SpawnShips() {
    if (!World->SelectedTarget.is_initialized()) {
        return;
    }
    if (World->SelectedPlanets.empty()) {
        return;
    }

    Space::TControl control;
    Space::TAttackCommand& attack = *control.mutable_attackcommand();
    for (auto& planet: World->SelectedPlanets) {
        attack.add_planetfrom(planet);
    }
    attack.set_planetto(*World->SelectedTarget);
    attack.set_energypercent(50);
    emit OnControl(control);
    LastSendControl.restart();
}
