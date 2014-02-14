#include <unordered_set>

#include <QDebug>
#include <qmath.h>

#include <projects/space_capture/lib/defines.h>

#include "world.h"

inline void Normalize(QPointF& point) {
    if (point.x() == 0 && point.y() == 0) {
        point.setX(rand() % 20 - 10);
        point.setY(rand() % 20 - 10);
    }
    float l = sqrt(point.x() * point.x() + point.y() * point.y());
    point.setX(point.x() / l);
    point.setY(point.y() / l);
}

inline float Distance(const QPointF& a, const QPointF& b) {
    return sqrt((a.x() - b.x()) * (a.x() - b.x()) + (a.y() - b.y()) * (a.y() - b.y()));
}

TWorld::TWorld(QObject *parent)
    : QObject(parent)
    , Time(0)
    , RoundStartsAt(-1)
{
    startTimer(50);
}

void TWorld::RestartRound() {
    Planets.clear();
    Ships.clear();
    qDebug() << "New round in" << ROUND_RESTART_TIME << "seconds";
    RoundStartsAt = ROUND_RESTART_TIME;
}

void TWorld::DoRestartRound() {
    qDebug() << "Round launched";
    Planets.clear();
    Ships.clear();
    GeneratePlayerPlanets();
    GenerateRandomPlanets();
}

bool TWorld::IsPossiblePlanetPosition(const QPointF& position, float radius, float minDistance) {
    if (position.x() < radius + 0.05 * WORLD_WIDTH || position.x() > WORLD_WIDTH - radius - 0.05 * WORLD_WIDTH) {
        return false;
    }
    if (position.y() < radius + 0.05 * WORLD_HEIGHT || position.y() > WORLD_HEIGHT - radius - 0.05 * WORLD_HEIGHT) {
        return false;
    }
    for (size_t i = 0; i < (size_t)Planets.size(); ++i) {
        TPlanet& planet = Planets[i];
        float distance = Distance(planet.Position, position);
        if (distance < planet.Radius + radius + minDistance) {
            return false;
        }
    }
    return true;
}

inline QPointF RandomPoint() {
    QPointF point;
    point.setX(rand() % (size_t)WORLD_WIDTH);
    point.setY(rand() % (size_t)WORLD_HEIGHT);
    return point;
}

void TWorld::GeneratePlayerPlanets() {
    for (auto& player: Players) {
        QPointF point = RandomPoint();
        while (!IsPossiblePlanetPosition(point, PLAYER_PLANET_RADIUS, MINIMUM_PLAYER_PLANET_DISTANCE)) {
            point = RandomPoint();
        }
        TPlanet planet;
        planet.Id = Planets.size();
        planet.Position = point;
        planet.PlayerId = player.Id;
        planet.Energy = PLAYER_PLANET_ENERGY;
        planet.Radius = PLAYER_PLANET_RADIUS;
        planet.Type = rand() % MAX_PLANET_TYPES;
        planet.SpawnCounter = 0;
        Planets[planet.Id] = planet;
    }
}

void TWorld::GenerateRandomPlanets() {
    size_t planetsNumber = PLANETS_MIN_NUMBER + rand() % (PLANETS_MAX_NUMBER - PLANETS_MIN_NUMBER);
    for (size_t i = 0; i < planetsNumber; ++i) {
        QPointF point = RandomPoint();
        float radius = PLANET_MIN_RADIUS + rand() % size_t(PLANET_MAX_RADIUS - PLANET_MIN_RADIUS);
        while (!IsPossiblePlanetPosition(point, radius, MINIMUM_PLANET_DISTANCE)) {
            point = RandomPoint();
            radius = PLANET_MIN_RADIUS + rand() % size_t(PLANET_MAX_RADIUS - PLANET_MIN_RADIUS);
        }
        TPlanet planet;
        planet.Id = Planets.size();
        planet.Position = point;
        planet.PlayerId = -1;
        planet.Energy = PLANET_MIN_ENERGY + rand() % size_t(PLANET_MAX_ENERGY - PLANET_MIN_ENERGY);
        planet.Radius = radius;
        planet.Type = rand() % MAX_PLANET_TYPES;
        planet.SpawnCounter = 0;
        Planets[planet.Id] = planet;
    }
}

void TWorld::UpdatePlanetEnergy() {
    for (size_t i = 0; i < (size_t)Planets.size(); ++i) {
        TPlanet& planet = Planets[i];
        if (planet.PlayerId == -1) {
            continue;   // skip neutral planets
        }
        float energy = planet.Radius / 10.0;
        planet.Energy += energy;
    }
}

inline void LimitSpeed(QPointF& speed, float limit = 3.5) {
    float speedAbs = Distance(QPointF(), speed);
    if (speedAbs > limit) {
        speed = (speed / speedAbs) * limit;
    }
}

void TWorld::ProcessShips() {
    QVector<TShip> newShips;
    newShips.reserve(Ships.size());

    for (size_t i = 0; i < (size_t)Ships.size(); ++i) {
        TShip& ship = Ships[i];
        QPointF v1 = Rule1(i);
        QPointF v2 = Rule2(i);
        QPointF v3 = Rule3(i);
        QPointF v4 = Rule4(i);
        QPointF v5 = Rule5(i);
        ship.Speed = ship.Speed + v1 + v2 + v3 + v4 + v5;
        LimitSpeed(ship.Speed);

        ship.Position.setX(ship.Position.x() + ship.Speed.x());
        ship.Position.setY(ship.Position.y() + ship.Speed.y());
        if (!ProcessCollision(ship) &&
            ship.Position.x() >= 0 && ship.Position.x() < WORLD_WIDTH &&
            ship.Position.y() >= 0 && ship.Position.y() < WORLD_HEIGHT)
        {
            newShips.push_back(ship);
        }
    }
    Ships.swap(newShips);
}

void TWorld::SendWorld() {
    Space::TWorld world;
    if (Time % 10 == 0) {
        for (size_t i = 0; i < (size_t)Planets.size(); ++i) {
            TPlanet* planet = &Planets[i];
            Space::TPlanet* packetPlanet = world.add_planets();
            packetPlanet->set_id(planet->Id);
            packetPlanet->set_playerid(planet->PlayerId);
            packetPlanet->set_radius(planet->Radius);
            packetPlanet->set_x(planet->Position.x());
            packetPlanet->set_y(planet->Position.y());
            packetPlanet->set_energy(planet->Energy);
            packetPlanet->set_type(planet->Type);
        }
        for (auto& player: Players) {
            Space::TPlayer* packetPlayer = world.add_players();
            packetPlayer->set_id(player.Id);
            packetPlayer->set_name(player.Name.toStdString());
            packetPlayer->set_color(player.Color);
        }
    }

    for (size_t i = 0; i < (size_t)Ships.size(); ++i) {
        TShip* ship = &Ships[i];
        Space::TShip* packetShip = world.add_ships();
        packetShip->set_playerid(ship->PlayerId);
        packetShip->set_x(ship->Position.x());
        packetShip->set_y(ship->Position.y());
        float angle = atan(ship->Speed.y() / ship->Speed.x());
        packetShip->set_angle(angle * 100);
    }

    if (Players.size() < 2) {
        world.set_waitingplayers(true);
    }

    if (RoundStartsAt >= 0) {
        world.set_roundstartsat(RoundStartsAt);
    }

    for (auto& player: Players) {
        if (Time % 10 == 0) {
            world.set_selfid(player.Id);
        }
        emit SendWorldToPlayer(world, player.Id);
    }
}

void TWorld::timerEvent(QTimerEvent *) {
    Time += 1;
    if (Players.size() >= 2) {
        if (RoundStartsAt > 0) {
            if (Time % 20 == 0) {
                --RoundStartsAt;
            }
        } else if (RoundStartsAt == 0) {
            DoRestartRound();
            RoundStartsAt = -1;
        } else {
            if (Time % 20 == 0) {
                UpdatePlanetEnergy(); // update energy every second
            }
            ProcessShips();
            ProcessShipSpawn();
            CheckRoundEnd();
        }
    }

    SendWorld();
}

void TWorld::SpawnShips(TPlanet& from, TPlanet& to, float energyPercents, size_t playerId, size_t maxShips) {
    float totalShipsEnergy = from.Energy * energyPercents;
    from.Energy -= totalShipsEnergy;
    size_t shipsCount = std::min(1 + (int)(totalShipsEnergy / 7.0), int(maxShips));
    float energyPerShip = totalShipsEnergy / shipsCount;
    QPointF direction;
    direction.setX(to.Position.x() - from.Position.x());
    direction.setY(to.Position.y() - from.Position.y());
    Normalize(direction);

    QPointF shipPosition = from.Position;
    shipPosition.setX(shipPosition.x() + direction.x() * (from.Radius + 6.0));
    shipPosition.setY(shipPosition.y() + direction.y() * (from.Radius + 6.0));
    QPointF shipSpeed = direction;
    shipSpeed.setX(shipSpeed.x() * 2.5);
    shipSpeed.setY(shipSpeed.y() * 2.5);

    for (size_t i = 0; i < shipsCount; ++i) {
        TShip ship;
        ship.Position = shipPosition;
        ship.Position.setX(ship.Position.x() + rand() % 6 - 3);
        ship.Position.setY(ship.Position.y() + rand() % 6 - 3);
        ship.Energy = energyPerShip;
        ship.PlayerId = playerId;
        ship.Speed = shipSpeed;
        ship.Target = to.Position;
        from.SpawnQueue.push_back(ship);
    }
}

void TWorld::ProcessShipSpawn() {
    for (size_t i = 0; i < (size_t)Planets.size(); ++i) {
        TPlanet& planet = Planets[i];
        if (!planet.SpawnQueue.empty()) {
            if (planet.SpawnCounter % 4 == 0) {
                Ships.push_back(planet.SpawnQueue.back());
                planet.SpawnQueue.pop_back();
                planet.SpawnCounter = 0;
            }
            ++planet.SpawnCounter;
        }
    }
}

bool TWorld::ProcessCollision(TShip& ship) {
    for (size_t i = 0; i < (size_t)Planets.size(); ++i) {
        TPlanet& planet = Planets[i];
        if (Distance(ship.Position, planet.Position) > planet.Radius) {
            continue;
        }
        if (planet.PlayerId == (int)ship.PlayerId) {
            planet.Energy += ship.Energy;
        } else if (planet.Energy > ship.Energy) {
            planet.Energy -= ship.Energy;
        } else {
            planet.Energy = ship.Energy - planet.Energy;
            planet.PlayerId = ship.PlayerId;
        }
        return true;
    }
    return false;
}

void TWorld::CheckRoundEnd() {
    std::unordered_set<int> playersLeft;
    for (size_t i = 0; i < (size_t)Planets.size(); ++i) {
        if (Planets[i].PlayerId == -1) {
            continue;
        }
        playersLeft.insert(Planets[i].PlayerId);
    }
    if (playersLeft.size() < 2) {
        RestartRound();
    }
}

size_t TWorld::GetPlayersCount() const {
    return Players.size();
}

bool TWorld::Empty() const {
    return Players.empty();
}

bool TWorld::Full() const {
    return Players.size() == MAX_PLAYERS;
}

void TWorld::OnNewPlayer(size_t playerId) {
    TPlayer player;
    player.Id = playerId;
    player.Name = "noob";
    player.Color = (Space::EColor)(playerId + 1);
    Players[player.Id] = player;
    if (Players.size() == 2) {
        RestartRound();
    }
}

void TWorld::OnPlayerLeft(size_t playerId) {
    qDebug() << "disconnected player" << playerId;
    for (auto& planet: Planets) {
        if (planet.PlayerId == playerId) {
            planet.PlayerId = -1;
            planet.SpawnQueue.clear();
        }
    }
    QVector<TShip> newShips;
    newShips.reserve(Ships.size());
    for (auto& ship: Ships) {
        if (ship.PlayerId != playerId) {
            newShips.push_back(ship);
        }
    }
    Ships.swap(newShips);
    Players.remove(playerId);
    if (Players.size() < 2) {
        RoundStartsAt = -1;
    }
}

void TWorld::OnControl(size_t playerId, Space::TControl control) {
    if (Players.find(playerId) == Players.end()) {
        qDebug() << "error: player not found, id: " << playerId;
        return;
    }
    TPlayer& player = Players[playerId];
    if (control.has_attackcommand()) {
        Attack(player, control.attackcommand());
    }
}

void TWorld::Attack(TPlayer& player, Space::TAttackCommand control) {
    std::vector<size_t> planetsFrom;
    planetsFrom.reserve(control.planetfrom_size());
    for (size_t i = 0; i < control.planetfrom_size(); ++i) {
        planetsFrom.push_back(control.planetfrom(i));
    }

    size_t planetTo = control.planetto();
    if (planetsFrom.empty()) {
        return;
    }

    if (Planets.find(planetTo) == Planets.end()) {
        return;
    }
    TPlanet& to = Planets[planetTo];

    size_t maxShips = std::min(int(40.0 / planetsFrom.size()), 14);

    for (size_t i = 0; i < planetsFrom.size(); ++i) {
        if (Planets.find(planetsFrom[i]) == Planets.end()) {
            continue;
        }
        TPlanet& from = Planets[planetsFrom[i]];
        if (from.PlayerId != (int)player.Id) {
            continue;
        }
        float energyPercents = 0.01 * control.energypercent();
        if (from.Energy * energyPercents >= 1.0) {
            SpawnShips(from, to, energyPercents, player.Id, maxShips);
        }
    }
}

QPointF TWorld::Rule1(size_t shipNum) {
    QPointF massCenter;
    size_t ships = 0;
    for (int i = 0; i < Ships.size(); ++i) {
        if (i != shipNum &&
            Ships[i].PlayerId == Ships[shipNum].PlayerId &&
            Ships[i].Target == Ships[shipNum].Target)
        {
            massCenter += Ships[i].Position;
            ++ships;
        }
    }
    if (!ships) {
        return QPointF(0, 0);
    }
    massCenter /= ships;
    return (massCenter - Ships[shipNum].Position) / 160;
}

QPointF TWorld::Rule2(size_t shipNum) {
    QPointF c(0, 0);
    for (int i = 0; i < Ships.size(); ++i) {
        if (i != shipNum) {
            float distance = Distance(Ships[i].Position, Ships[shipNum].Position);
            if (distance < 21) {
                QPointF direction = Ships[i].Position - Ships[shipNum].Position;
                Normalize(direction);
                direction *= (33 - distance);
                c -= direction;
            }
        }
    }
    return c / 35;
}

QPointF TWorld::Rule3(size_t shipNum) {
    QPointF massCenter;
    size_t ships = 0;
    for (int i = 0; i < Ships.size(); ++i) {
        if (i != shipNum &&
            Ships[i].PlayerId == Ships[shipNum].PlayerId &&
            Ships[i].Target == Ships[shipNum].Target)
        {
            massCenter += Ships[i].Speed;
            ++ships;
        }
    }
    if (!ships) {
        return QPointF(0, 0);
    }
    massCenter /= ships;
    return (massCenter - Ships[shipNum].Speed) / 18;
}

QPointF TWorld::Rule4(size_t shipNum) {
    QPointF direction = Ships[shipNum].Target - Ships[shipNum].Position;
    LimitSpeed(direction, 0.85);
    return direction;
}

QPointF TWorld::Rule5(size_t shipNum) {
    QPointF c(0, 0);
    for (int i = 0; i < Planets.size(); ++i) {
        if (Ships[shipNum].Target != Planets[i].Position) {
            float distance = Distance(Planets[i].Position, Ships[shipNum].Position);
            if (distance < 10.0 + 1.4 * Planets[i].Radius) {
                QPointF direction = Planets[i].Position - Ships[shipNum].Position;
                Normalize(direction);
                direction *= (18.0 + 1.4 * Planets[i].Radius - distance);
                c -= direction;
            }
        }
    }
    return c / 8;
}
