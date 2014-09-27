#pragma once

#include <unordered_map>
#include <unordered_set>
#include <functional>

#include "../../lib/space.h"
#include "../../lib/defines.h"

struct TPointF {
    float X;
    float Y;
};

struct TPoint {
    int X;
    int Y;
};

struct TSize {
    int Width;
    int Height;
};

struct TSelection {
    TPoint From;
    TPoint To;
};

class TWorld: public NSpace::TWorld {
public:
    void UpdateWorld(const NSpace::TWorld& world);
    void UpdateSelection(TPoint from, TPoint to);
    void RemoveSelection();
    NSpace::TPlayer* SelfPlayer();
public:
    std::function<void()> OnWorldUpdated;
    float Scale = 0.65;
    float OffsetX = 0;
    float OffsetY = 0;
    std::unordered_map<size_t, NSpace::TPlayer*> IdToPlayer;
    std::unordered_set<size_t> SelectedPlanets;
    size_t SelectedTarget = -1;
    std::string PlayerName;
    TSelection Selection;
    bool HaveSelection;
    int Power = 50; // percents
    int ScreenWidth = 0;
    int ScreenHeight = 0;
    float Ratio = 0;
};
