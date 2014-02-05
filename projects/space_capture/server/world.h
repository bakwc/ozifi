#pragma once

#include <QObject>
#include <QVector>
#include <QHash>
#include <QPointF>

#include <projects/space_capture/lib/space.pb.h>

struct TPlayer {
    QString Name;
    size_t Id;
    Space::EColor Color;
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
const float MINIMUM_PLANET_DISTANCE = 15.0f;
const float PLAYER_PLANET_ENERGY = 100.0f;
const size_t PLANETS_MAX_NUMBER = 20;
const size_t PLANETS_MIN_NUMBER = 10;
const float PLANET_MAX_RADIUS = 60.0f;
const float PLANET_MIN_RADIUS = 10.0f;
const float PLANET_MAX_ENERGY = 50.0f;
const float PLANET_MIN_ENERGY = 0.0f;

class TWorld : public QObject
{
    Q_OBJECT
public:
    TWorld(QObject *parent = 0);
    void RestartRound();
    bool IsPossiblePlanetPosition(const QPointF& position, float radius, float minDistance);
    void GeneratePlayerPlanets();
    void GenerateRandomPlanets();
    void UpdatePlanetEnergy();
    void ProcessShips();
    void SendWorld();
    void timerEvent(QTimerEvent*);
    void SpawnShips(TPlanet& from, TPlanet& to, float energyPercents, size_t playerId);
    void ProcessShipSpawn();
    bool ProcessCollision(TShip& ship);
    void CheckRoundEnd();
signals:
    void SendWorldToPlayer(Space::TWorld world, size_t playerId);
public slots:
    void OnNewPlayer(size_t playerId);
    void OnControl(size_t playerId, Space::TControl control);
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
    int Power;
};
