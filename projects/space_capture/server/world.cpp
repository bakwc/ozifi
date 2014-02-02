#include <unordered_set>

#include <QDebug>
#include <qmath.h>

#include <projects/space_capture/lib/defines.h>

#include "world.h"

inline void Normalize(QPointF& point) {
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
{
    startTimer(50);
}

void TWorld::RestartRound() {
    qDebug() << "Round restarted";
    Planets.clear();
    GeneratePlayerPlanets();
    GenerateRandomPlanets();
}

bool TWorld::IsPossiblePlanetPosition(const QPointF& position, float radius, float minDistance) {
    if (position.x() < radius || position.x() > WORLD_WIDTH - radius) {
        return false;
    }
    if (position.y() < radius || position.y() > WORLD_HEIGHT - radius) {
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
    for (size_t i = 0; i < (size_t)Players.size(); ++i) {
        QPointF point = RandomPoint();
        while (!IsPossiblePlanetPosition(point, PLAYER_PLANET_RADIUS, MINIMUM_PLAYER_PLANET_DISTANCE)) {
            point = RandomPoint();
        }
        TPlanet planet;
        planet.Id = Planets.size();
        planet.Position = point;
        planet.PlayerId = Players[i].Id;
        planet.Energy = PLAYER_PLANET_ENERGY;
        planet.Radius = PLAYER_PLANET_RADIUS;
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

void TWorld::ProcessShips() {
    QVector<TShip> newShips;
    newShips.reserve(Ships.size());
    for (size_t i = 0; i < (size_t)Ships.size(); ++i) {
        TShip& ship = Ships[i];
        ship.Position.setX(ship.Position.x() + ship.Speed.x());
        ship.Position.setY(ship.Position.y() + ship.Speed.y());
        if (!ProcessCollision(ship)) {
            newShips.push_back(ship);
        }
    }
    Ships.swap(newShips);
}

void TWorld::SendWorld() {
    Space::TWorld world;
    for (size_t i = 0; i < (size_t)Planets.size(); ++i) {
        TPlanet* planet = &Planets[i];
        Space::TPlanet* packetPlanet = world.add_planets();
        packetPlanet->set_id(planet->Id);
        packetPlanet->set_playerid(planet->PlayerId);
        packetPlanet->set_radius(planet->Radius);
        packetPlanet->set_x(planet->Position.x());
        packetPlanet->set_y(planet->Position.y());
        packetPlanet->set_energy(planet->Energy);
    }
    for (size_t i = 0; i < (size_t)Ships.size(); ++i) {
        TShip* ship = &Ships[i];
        Space::TShip* packetShip = world.add_ships();
        packetShip->set_playerid(ship->PlayerId);
        packetShip->set_x(ship->Position.x());
        packetShip->set_y(ship->Position.y());
    }
    for (size_t i = 0; i < (size_t)Players.size(); ++i) {
        TPlayer* player = &Players[i];
        Space::TPlayer* packetPlayer = world.add_players();
        packetPlayer->set_id(player->Id);
        packetPlayer->set_name(player->Name.toStdString());
        packetPlayer->set_color(player->Color);
    }
    for (auto& player: Players) {
        world.set_selfid(player.Id);
        emit SendWorldToPlayer(world, player.Id);
    }
}

void TWorld::timerEvent(QTimerEvent *) {
    Time += 1;
    if (Players.size() < 2) {
        return; // rount not started yet
    }
    if (Time % 20 == 0) {
        UpdatePlanetEnergy(); // update energy every second
    }
    ProcessShips();
    CheckRoundEnd();
    SendWorld();
}

void TWorld::SpawnShips(TPlanet& from, TPlanet& to, float energyPercents, size_t playerId) {
    float totalShipsEnergy = from.Energy * energyPercents;
    from.Energy -= totalShipsEnergy;
    size_t shipsCount = 1 + totalShipsEnergy / 7.0;
    float energyPerShip = totalShipsEnergy / shipsCount;
    QPointF direction;
    direction.setX(to.Position.x() - from.Position.x());
    direction.setY(to.Position.y() - from.Position.y());
    Normalize(direction);

    QPointF shipPosition = from.Position;
    shipPosition.setX(shipPosition.x() + direction.x() * from.Radius);
    shipPosition.setY(shipPosition.y() + direction.y() * from.Radius);
    QPointF shipSpeed = direction;
    shipSpeed.setX(shipSpeed.x() * 2.5);
    shipSpeed.setY(shipSpeed.y() * 2.5);

    for (size_t i = 0; i < shipsCount; ++i) {
        TShip ship;
        ship.Position = shipPosition;
        ship.Energy = energyPerShip;
        ship.PlayerId = playerId;
        ship.Speed = shipSpeed;
        shipPosition.setX(shipPosition.x() + direction.x() * 6.0);
        shipPosition.setY(shipPosition.y() + direction.y() * 6.0);
        Ships.push_back(ship);
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
    if (playersLeft.size() == 1) {
        // todo: lobby
        RestartRound();
    }
}

void TWorld::OnNewPlayer(size_t playerId) {
    TPlayer player;
    player.Id = playerId;
    player.Name = "noob";
    player.Color = (Space::EColor)(playerId + 1);
    Players[player.Id] = player;
    if (Players.size() >= 2) {
        RestartRound();
    }
}

void TWorld::OnControl(size_t playerId, Space::TControl control) {
    if (Players.find(playerId) == Players.end()) {
        qDebug() << "error: player not found, id: " << playerId;
        return;
    }
    TPlayer& player = Players[playerId];
    std::vector<size_t> planetsFrom;
    planetsFrom.reserve(control.planetfrom_size());
    for (size_t i = 0; i < control.planetfrom_size(); ++i) {
        planetsFrom.push_back(control.planetfrom(i));
    }

    size_t planetTo = control.planetto();
    if (control.has_playername()) {
        player.Name = QString::fromStdString(control.playername());
    }
    if (planetsFrom.empty()) {
        return;
    }

    if (Planets.find(planetTo) == Planets.end()) {
        return;
    }
    TPlanet& to = Planets[planetTo];

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
            SpawnShips(from, to, energyPercents, playerId);
        }
    }
}
