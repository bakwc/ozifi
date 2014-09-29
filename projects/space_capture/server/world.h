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
