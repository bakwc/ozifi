#pragma once

#include "gameplay.h"

class IDrawable {
public:
    virtual void Draw(float) = 0;
};

class IControlable {
public:
    virtual void OnTouchEvent(gameplay::Touch::TouchEvent evt, int x, int y, unsigned int contactIndex) {
    }
    virtual void OnResized(size_t width, size_t heigth) {
    }
    virtual void OnWheelEvent(int wheelDelta) {
    }
};
