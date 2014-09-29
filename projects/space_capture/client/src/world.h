#pragma once

#include <unordered_map>
#include <unordered_set>
#include <functional>

#include "../../lib/world.h"
#include "../../lib/defines.h"

struct TSelection {
    TPoint From;
    TPoint To;
};

class TWorld: public NSpaceEngine::TWorld {
public:
    TWorld(std::function<void(const std::string &)> onCommand);
    void UpdateWorld(const std::string& data);
    void OnCommandReceived(const std::string& command);
    void UpdateSelection(TPoint from, TPoint to);
    void RemoveSelection();
    NSpaceEngine::TPlayer* SelfPlayer();
public:
    std::function<void()> OnWorldUpdated;
    float Scale = 0.65;
    float OffsetX = 0;
    float OffsetY = 0;
    std::unordered_set<size_t> SelectedPlanets;
    size_t SelectedTarget = -1;
    std::string PlayerName;
    TSelection Selection;
    bool HaveSelection;
    int Power = 50; // percents
    int ScreenWidth = 0;
    int ScreenHeight = 0;
    float Ratio = 0;
    size_t SelfId = 0;
};
