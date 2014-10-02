#pragma once

#include <chrono>

#include "world.h"
#include "display.h"

enum EControlState {
    CS_None,
    CS_PlanetSelection,
    CS_TargetSelection
};

using TControlFn = std::function<void(const std::vector<uint8_t>& planetsFrom,
                                      uint8_t planetTo,
                                      uint8_t energyPercent)>;

class TApplication;
class TControl: public IControlable {
public:
    explicit TControl(TWorld* world,
                      TControlFn onControl,
                      TApplication *app);
public:
    void OnTouchEvent(gameplay::Touch::TouchEvent evt, int x, int y, unsigned int contactIndex) override;
    void OnResized(size_t width, size_t height) override;
    void OnWheelEvent(int wheelDelta) override;
    void OnDoubleClick();
private:
    void OnMouseEvent(TPoint pos, bool mouseDown);
    void OnMouseMove(TPoint pos);
    void CheckPower(TPoint pos);
private:
    void CheckSelection(TPoint from, TPoint to);
    void CheckTargetSelection(TPoint position);
    void SpawnShips();
private:
    TWorld* World;
    EControlState State;
    TPoint SelectionFrom;
    TControlFn OnControl;
    TApplication* Application;
    std::chrono::system_clock::time_point LastClick;
    int Width;
    int Height;
    bool Pressed = false;
};
