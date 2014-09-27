#include "control.h"
#include "TApplication.h"

TControl::TControl(TWorld *world,
                   std::function<void(NSpace::TAttackCommand)> onControl,
                   TApplication* app)
    : World(world)
    , State(CS_None)
    , OnControl(onControl)
    , Application(app)
{
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

void TControl::CheckSelection(TPoint from, TPoint to) {
    World->SelectedPlanets.clear();
    int ax1 = from.X;
    int ay1 = from.Y;
    int ax2 = to.X;
    int ay2 = to.Y;
    for (size_t i = 0; i < World->Planets.size(); ++i) {
        if (World->Planets[i].PlayerId != World->SelfId) {
            continue;
        }
        float r = World->Planets[i].Radius;
        float bx1 = 0.01 * World->Scale * (World->Planets[i].X - r - 0.5 * WORLD_WIDTH);
        float bx2 = 0.01 * World->Scale * (World->Planets[i].X + r - 0.5 * WORLD_WIDTH);
        float by1 = 0.01 * World->Scale * (World->Planets[i].Y - r - 0.5 * WORLD_HEIGHT);
        float by2 = 0.01 * World->Scale * (World->Planets[i].Y + r - 0.5 * WORLD_HEIGHT);
        float bbx1, bby1, bbx2, bby2;
        Application->project(gameplay::Vector3(bx1, by1, -10), bbx1, bby1);
        Application->project(gameplay::Vector3(bx2, by2, -10), bbx2, bby2);
        bby1 = Application->getHeight() - bby1;
        bby2 = Application->getHeight() - bby2;

        if (Intersects(ax1, ax2, ay1, ay2, bbx1, bbx2, bby1, bby2)) {
            World->SelectedPlanets.insert(World->Planets[i].ID);
        }
    }
}

void TControl::CheckTargetSelection(TPoint position) {
    int ax1 = position.X;
    int ax2 = position.X;
    int ay1 = position.Y;
    int ay2 = position.Y;

    for (size_t i = 0; i < World->Planets.size(); ++i) {
        float r = World->Planets[i].Radius;
        float bx1 = 0.01 * World->Scale * (World->Planets[i].X - r - 0.5 * WORLD_WIDTH);
        float bx2 = 0.01 * World->Scale * (World->Planets[i].X + r - 0.5 * WORLD_WIDTH);
        float by1 = 0.01 * World->Scale * (World->Planets[i].Y - r - 0.5 * WORLD_HEIGHT);
        float by2 = 0.01 * World->Scale * (World->Planets[i].Y + r - 0.5 * WORLD_HEIGHT);
        float bbx1, bby1, bbx2, bby2;
        Application->project(gameplay::Vector3(bx1, by1, -10), bbx1, bby1);
        Application->project(gameplay::Vector3(bx2, by2, -10), bbx2, bby2);
        bby1 = Application->getHeight() - bby1;
        bby2 = Application->getHeight() - bby2;

        if (Intersects(ax1, ax2, ay1, ay2, bbx1, bbx2, bby1, bby2)) {
            World->SelectedTarget = i;
            return;
        }
    }
    World->SelectedTarget = -1;
}

void TControl::SpawnShips() {
    if (!World->Power) {
        return;
    }

    if (World->SelectedTarget == size_t(-1)) {
        return;
    }
    if (World->SelectedPlanets.empty()) {
        return;
    }

    NSpace::TAttackCommand attack;
    for (auto& planet: World->SelectedPlanets) {
        attack.PlanetFrom.push_back(planet);
    }
    attack.PlanetTo = World->SelectedTarget;
    attack.EnergyPercent = World->Power;
    OnControl(attack);
}

void TControl::OnTouchEvent(gameplay::Touch::TouchEvent evt,
                            int x, int y, unsigned int contactIndex)
{
    TPoint pos = {x, (int)Application->getHeight() - y};
    if (evt == gameplay::Touch::TOUCH_PRESS) {
        OnMouseEvent(pos, true);
    } else if (evt == gameplay::Touch::TOUCH_RELEASE) {
        OnMouseEvent(pos, false);
    } else if (evt == gameplay::Touch::TOUCH_MOVE) {
        OnMouseMove(pos);
    }
}

void TControl::OnResized(size_t width, size_t heigth) {
}

void TControl::OnWheelEvent(int wheelDelta) {
}

void TControl::OnMouseEvent(TPoint pos, bool mouseDown) {
    if (State == CS_None) {
        CheckSelection(pos, pos);
        if (!World->SelectedPlanets.empty()) {
            State = CS_TargetSelection;
        } else if (mouseDown) {
            State = CS_PlanetSelection;
            SelectionFrom = pos;
        }
    } else if (State == CS_PlanetSelection && !mouseDown) {
        if (!World->SelectedPlanets.empty()) {
            State = CS_TargetSelection;
            World->HaveSelection = false;
        } else {
            State = CS_None;
            World->SelectedPlanets.clear();
            World->HaveSelection = false;
            World->SelectedTarget = size_t(-1);
        }
    } else if (State == CS_TargetSelection && !mouseDown) {
        CheckTargetSelection(pos);
        if (World->SelectedTarget != size_t(-1)) {
            if (World->SelectedPlanets.size() == 1 &&
                *World->SelectedPlanets.begin() == World->SelectedTarget)
            {
                CheckSelection(pos, pos);
            } else {
                SpawnShips();
                State = CS_None;
                World->SelectedPlanets.clear();
                World->HaveSelection = false;
                World->SelectedTarget = size_t(-1);
            }
        } else {
            State = CS_None;
            World->SelectedPlanets.clear();
            World->HaveSelection = false;
            World->SelectedTarget = size_t(-1);
        }
    }
}

void TControl::OnMouseMove(TPoint pos) {
    if (State == CS_PlanetSelection) {
        World->UpdateSelection(SelectionFrom, pos);
        CheckSelection(SelectionFrom, pos);
    } else if (State == CS_TargetSelection) {
        CheckTargetSelection(pos);
    }
}

