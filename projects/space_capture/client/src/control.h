#pragma once

#include "world.h"
#include "display.h"

enum EControlState {
    CS_None,
    CS_PlanetSelection,
    CS_TargetSelection
};

class TApplication;
class TControl: public IControlable {
public:
    explicit TControl(TWorld* world,
                      std::function<void(NSpace::TAttackCommand)> onControl,
                      TApplication *app);
public:
    void OnTouchEvent(gameplay::Touch::TouchEvent evt, int x, int y, unsigned int contactIndex) override;
    void OnResized(size_t width, size_t heigth) override;
    void OnWheelEvent(int wheelDelta) override;
private:
    void OnMouseEvent(TPoint pos, bool mouseDown);
    void OnMouseMove(TPoint pos);
private:
    void CheckSelection(TPoint from, TPoint to);
    void CheckTargetSelection(TPoint position);
    void SpawnShips();
private:
    TWorld* World;
    EControlState State;
    TPoint SelectionFrom;
    std::function<void(NSpace::TAttackCommand)> OnControl;
    TApplication* Application;
};
