#pragma once

#include <QObject>
#include <QVector>
#include <QHash>
#include <QPointF>

#include <projects/space_capture/lib/space.h>

struct TPlayer {
    QString Name;
    size_t Id;
    NSpace::EColor Color;
    bool NeedFullUpdate;
};

struct TShip {
    QPointF Position;
    QPointF Speed;
    QPointF Target;
    float Energy;
    size_t PlayerId;
};

struct TPlanet {
    size_t Id;
    QPointF Position;
    float Radius;
    float Energy;
    int PlayerId;
    int Type;
    std::vector<TShip> SpawnQueue;
    size_t SpawnCounter;
};

const float PLAYER_PLANET_RADIUS = 30.0f;
const float MINIMUM_PLAYER_PLANET_DISTANCE = 100.0f;
const float MINIMUM_PLANET_DISTANCE = 31.0f;
const float PLAYER_PLANET_ENERGY = 100.0f;
const size_t PLANETS_MAX_NUMBER = 20;
const size_t PLANETS_MIN_NUMBER = 10;
const float PLANET_MAX_RADIUS = 50.0f;
const float PLANET_MIN_RADIUS = 10.0f;
const float PLANET_MAX_ENERGY = 50.0f;
const float PLANET_MIN_ENERGY = 0.0f;
const size_t ROUND_RESTART_TIME = 7;

class TWorld : public QObject
{
    Q_OBJECT
public:
    TWorld(QObject *parent = 0);
    void RestartRound();
    void DoRestartRound();
    bool IsPossiblePlanetPosition(const QPointF& position, float radius, float minDistance);
    void GeneratePlayerPlanets();
    void GenerateRandomPlanets();
    void UpdatePlanetEnergy();
    void ProcessShips();
    void SendWorld();
    void timerEvent(QTimerEvent*);
    void SpawnShips(TPlanet& from, TPlanet& to, float energyPercents, size_t playerId, size_t maxShips = 18);
    void ProcessShipSpawn();
    bool ProcessCollision(TShip& ship);
    void CheckRoundEnd();
public:
    size_t GetPlayersCount() const;
    bool Empty() const;
    bool Full() const;
signals:
    void SendWorldToPlayer(NSpace::TWorld world, size_t playerId);
public slots:
    void OnNewPlayer(size_t playerId);
    void OnPlayerLeft(size_t playerId);
    void OnControl(size_t playerId, NSpace::TAttackCommand control);
private:
    void Attack(TPlayer& player, NSpace::TAttackCommand control);
private:
    QPointF Rule1(size_t shipNum);
    QPointF Rule2(size_t shipNum);
    QPointF Rule3(size_t shipNum);
    QPointF Rule4(size_t shipNum); // target
    QPointF Rule5(size_t shipNum); // avoid planets
private:
    QHash<size_t, TPlanet> Planets;
    QHash<size_t, TPlayer> Players;
    QVector<TShip> Ships;
    QPointF MassCenter;
    quint64 Time;
    int RoundStartsAt;
};
