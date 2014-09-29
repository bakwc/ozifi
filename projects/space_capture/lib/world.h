#pragma once

#include <iostream>
#include <string>
#include <map>
#include <random>
#include <functional>

#include <inttypes.h>

#include "defines.h"
#include "space.h"

namespace NSpaceEngine {

struct TPlayer {
    std::string Name;
    uint8_t Id;
    NSpace::EColor Color;
    bool NeedFullUpdate;

    SAVELOAD(Name, Id, Color, NeedFullUpdate)
};

struct TShip {
    TPointF Position;
    TPointF Speed;
    TPointF Target;
    float Energy;
    uint8_t PlayerId;
    float GetAngle() const {
        return atan(Speed.Y / Speed.X);
    }

    SAVELOAD(Position, Speed, Target, Energy, PlayerId)
};

struct TPlanet {
    uint8_t Id;
    TPointF Position;
    float Radius;
    float Energy;
    int PlayerId;
    int Type;
    std::vector<TShip> SpawnQueue;
    uint8_t SpawnCounter;

    SAVELOAD(Id, Position, Radius, Energy, PlayerId, Type, SpawnQueue)
};

class TWorld {
public:
    TWorld(bool isServer,
           std::function<void(const std::string& command)> onCommand =
            std::function<void(const std::string& command)>());
public:
    void Attack(uint8_t playerId,
                const std::vector<uint8_t>& planetsFrom,
                uint8_t planetTo,
                uint8_t energyPercent);

    void OnNewPlayer(uint8_t playerId);
    void OnPlayerLeft(uint8_t playerId);

    void Process();

    void PlayCommand(const std::string& cmd);
public:
    size_t GetPlayersCount() const;
    bool Empty() const;
    bool Full() const;
    std::string Serialize();
    void Deserialize(const std::string& data);
private:
    void DoRestartRound();

    void DoAttack(uint8_t playerId,
                  const std::vector<uint8_t>& planetsFrom,
                  uint8_t planetTo,
                  uint8_t energyPercent);

    void DoOnNewPlayer(uint8_t playerId);
    void DoOnPlayerLeft(uint8_t playerId);

    void DoProcess();

    void PlayCommand(std::istream& in);
private:
    void StartRound();
    void GeneratePlayerPlanets();
    void GenerateRandomPlanets();
    TPointF RandomPoint();
    bool IsPossiblePlanetPosition(const TPointF& position, float radius, float minDistance);
    void Normalize(TPointF& point);
    void UpdatePlanetEnergy();
    bool ProcessCollision(TShip& ship) ;
    void ProcessShips();
    void SpawnShips(TPlanet& from, TPlanet& to, float energyPercents,
                    size_t playerId, size_t maxShips);
    void ProcessShipSpawn();
    void CheckRoundEnd() ;
    TPointF Rule5(size_t shipNum);
    TPointF Rule4(size_t shipNum);
    TPointF Rule3(size_t shipNum);
    TPointF Rule2(size_t shipNum);
    TPointF Rule1(size_t shipNum);
public:
    bool IsServer;
    std::function<void(const std::string& command)> OnCommand;
    std::map<uint8_t, TPlanet> Planets;
    std::map<uint8_t, TPlayer> Players;
    std::vector<TShip> Ships;
    TPointF MassCenter;
    uint64_t Time;
    int RoundStartsAt;
    std::mt19937_64 Generator;
};

}
