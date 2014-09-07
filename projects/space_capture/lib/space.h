#pragma once

#include <vector>
#include <cinttypes>
#include <string>

#include "../../utils/saveload.h"

namespace NSpace {

struct TAttackCommand {
    std::vector<uint8_t> PlanetFrom;
    uint8_t PlanetTo = -1;
    uint8_t EnergyPercent = -1; // 0 - 100
    std::string PlayerName;

    SAVELOAD(PlanetFrom, PlanetTo, EnergyPercent, PlayerName)
};

struct TPlanet {
    uint8_t ID = -1;
    int32_t X = 0;
    int32_t Y = 0;
    uint32_t Radius = -1;
    uint32_t PlayerId = -1;
    uint32_t Energy = -1;
    uint32_t Type = -1;

    SAVELOAD(ID, X, Y, Radius, PlayerId, Energy, Type)
};

struct TShip {
    int32_t X = 0;
    int32_t Y = 0;
    uint32_t PlayerID = -1;
    int32_t Angle = 0;

    SAVELOAD(X, Y, PlayerID, Angle)
};

enum EColor {
    CR_Blue = 1,
    CR_Red = 2,
    CR_Green = 3,
    CR_Yellow = 4,
    CR_White = 5,
    CR_Cyan = 6
};
}

SAVELOAD_POD(NSpace::EColor)

namespace NSpace {

struct TPlayer {
    uint32_t ID = -1;
    std::string Name;
    EColor Color = CR_Blue;

    SAVELOAD(ID, Name, Color)
};

struct TWorld {
    std::vector<TPlanet> Planets;
    std::vector<TShip> Ships;
    std::vector<TPlayer> Players;
    uint32_t SelfId = -1;
    uint8_t RoundStartsAt = -1;
    bool WaitingPlayers = false;

    SAVELOAD(Planets, Ships, Players, SelfId, RoundStartsAt, WaitingPlayers)

    TWorld& operator=(const TWorld&) = default;
};

}
