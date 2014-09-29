#include <sstream>
#include <unordered_set>
#include <cassert>

#include "../../utils/saveload.h"

#include "world.h"

namespace NSpaceEngine {
enum EWorldCommand {
    CM_Process,
    CM_Attack,
    CM_NewPlayer,
    CM_PlayerLeft
};
}
SAVELOAD_POD(NSpaceEngine::EWorldCommand)

namespace NSpaceEngine {

using namespace std;

TWorld::TWorld(bool isServer,
               std::function<void(const string &)> onCommand)
    : IsServer(isServer)
    , OnCommand(onCommand)
{
}

void TWorld::Process() {
    stringstream out;
    ::Save(out, CM_Process);
    OnCommand(out.str());
    if (IsServer) {
        DoProcess();
    }
}

void TWorld::PlayCommand(const string& cmd) {
    imemstream in(cmd.data(), cmd.size());
    PlayCommand(in);
    if (IsServer) {
        OnCommand(cmd);
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

std::string TWorld::Serialize() {
    stringstream out;
    ::SaveMany(out, Planets, Players, Ships, MassCenter, Time, GroupsCounter, RoundStartsAt, Generator);
    return out.str();
}

void TWorld::Deserialize(const std::string& data) {
    imemstream in(data.data(), data.size());
    ::LoadMany(in, Planets, Players, Ships, MassCenter, Time, GroupsCounter, RoundStartsAt, Generator);
}

void TWorld::DoRestartRound() {
    Time = 0;
    GroupsCounter = 0;
    Planets.clear();
    Ships.clear();
    cout << "[INFO] New round in" << ROUND_RESTART_TIME << "seconds\n";
    RoundStartsAt = ROUND_RESTART_TIME;
}

void TWorld::DoAttack(uint8_t playerId,
                      const std::vector<uint8_t>& planetsFrom,
                      uint8_t planetTo,
                      uint8_t energyPercent)
{
    if (Players.find(playerId) == Players.end()) {
        return;
    }

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
            return;
        }
        TPlanet& from = Planets[planetsFrom[i]];
        if (from.PlayerId != (int)playerId) {
            return;
        }
        float energyPercentF = std::max(0.01f * energyPercent, 1.f);
        if (from.Energy * energyPercentF >= 1.0) {
            SpawnShips(from, to, energyPercentF, playerId, maxShips);
        }
    }
}

void TWorld::DoOnNewPlayer(uint8_t playerId) {
    cout << "[INFO] player entered " << (int)playerId << "\n";
    TPlayer player;
    player.Id = playerId;
    player.Name = "noob";
    player.Color = (NSpace::EColor)(playerId + 1);
    Players[player.Id] = player;
    if (Players.size() == 2) {
        DoRestartRound();
    }
}

void TWorld::DoOnPlayerLeft(uint8_t playerId) {
    cout << "[INFO] player left " << (int)playerId << "\n";
    for (auto&& planet: Planets) {
        if (planet.second.PlayerId == playerId) {
            planet.second.PlayerId = -1;
            planet.second.SpawnQueue.clear();
        }
    }
    std::vector<TShip> newShips;
    newShips.reserve(Ships.size());
    for (auto& ship: Ships) {
        if (ship.PlayerId != playerId) {
            newShips.push_back(ship);
        }
    }
    Ships.swap(newShips);
    Players.erase(playerId);
}

void TWorld::Attack(uint8_t playerId,
                      const std::vector<uint8_t>& planetsFrom,
                      uint8_t planetTo,
                      uint8_t energyPercent)
{
    stringstream out;
    ::SaveMany(out, CM_Attack, playerId, planetsFrom, planetTo, energyPercent);
    OnCommand(out.str());
    if (IsServer) {
        DoAttack(playerId, planetsFrom, planetTo, energyPercent);
    }
}

void TWorld::OnNewPlayer(uint8_t playerId) {
    stringstream out;
    ::SaveMany(out, CM_NewPlayer, playerId);
    OnCommand(out.str());
    if (IsServer) {
        DoOnNewPlayer(playerId);
    }
}

void TWorld::OnPlayerLeft(uint8_t playerId) {
    stringstream out;
    ::SaveMany(out, CM_PlayerLeft, playerId);
    OnCommand(out.str());
    if (IsServer) {
        DoOnPlayerLeft(playerId);
    }
}

void TWorld::DoProcess() {

    Time += 1;

    if (Players.size() < 2) {
        return;
    }

    if (RoundStartsAt > 0) {
        if (Time % 20 == 0) {
            --RoundStartsAt;
        }
    } else if (RoundStartsAt == 0) {
        StartRound();
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

void TWorld::PlayCommand(std::istream& in) {
   EWorldCommand cmd;
   ::Load(in, cmd);
   switch (cmd) {
   case CM_Process: {
       DoProcess();
   } break;
   case CM_Attack: {
        uint8_t playerId;
        std::vector<uint8_t> planetsFrom;
        uint8_t planetTo;
        uint8_t energyPercent;
        ::LoadMany(in, playerId, planetsFrom, planetTo, energyPercent);
        DoAttack(playerId, planetsFrom, planetTo, energyPercent);
   } break;
   case CM_NewPlayer: {
       uint8_t playerId;
       ::Load(in, playerId);
       DoOnNewPlayer(playerId);
   } break;
   case CM_PlayerLeft: {
       uint8_t playerId;
       ::Load(in, playerId);
       DoOnPlayerLeft(playerId);
   } break;
   default:
       throw std::string("unknown cmd");
   }
}

void TWorld::StartRound() {
    Planets.clear();
    Ships.clear();
    GeneratePlayerPlanets();
    GenerateRandomPlanets();
    std::cout << "[INFO] Round started\n";
}

TPointF TWorld::RandomPoint() {
    TPointF point;
    point.X = Generator() % (size_t)WORLD_WIDTH;
    point.Y = Generator() % (size_t)WORLD_HEIGHT;
    return point;
}

void TWorld::Normalize(TPointF& point) {
    if (point.X == 0 && point.Y == 0) {
        point.X = Generator() % 20 - 10;
        point.Y = Generator() % 20 - 10;
    }
    float l = sqrt(point.X * point.X + point.Y * point.Y);
    point.X /= l;
    point.Y /= l;
}

inline float Distance(const TPointF& a, const TPointF& b) {
    return sqrt((a.X - b.X) * (a.X - b.X) + (a.Y - b.Y) * (a.Y - b.Y));
}

bool TWorld::IsPossiblePlanetPosition(const TPointF& position, float radius, float minDistance) {
    if (position.X < radius + 0.05 * WORLD_WIDTH || position.X > WORLD_WIDTH - radius - 0.05 * WORLD_WIDTH) {
        return false;
    }
    if (position.Y < radius + 0.05 * WORLD_HEIGHT || position.Y > WORLD_HEIGHT - radius - 0.05 * WORLD_HEIGHT) {
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

void TWorld::GeneratePlayerPlanets() {
    for (auto& player: Players) {
        TPointF point = RandomPoint();
        while (!IsPossiblePlanetPosition(point, PLAYER_PLANET_RADIUS, MINIMUM_PLAYER_PLANET_DISTANCE)) {
            point = RandomPoint();
        }
        TPlanet planet;
        planet.Id = Planets.size();
        planet.Position = point;
        planet.PlayerId = player.second.Id;
        planet.Energy = PLAYER_PLANET_ENERGY;
        planet.Radius = PLAYER_PLANET_RADIUS;
        planet.Type = Generator() % MAX_PLANET_TYPES;
        planet.SpawnCounter = 0;
        Planets[planet.Id] = planet;
    }
}

void TWorld::GenerateRandomPlanets() {
    size_t planetsNumber = PLANETS_MIN_NUMBER + Generator() % (PLANETS_MAX_NUMBER - PLANETS_MIN_NUMBER);
    for (size_t i = 0; i < planetsNumber; ++i) {
        TPointF point = RandomPoint();
        float radius = PLANET_MIN_RADIUS + Generator() % size_t(PLANET_MAX_RADIUS - PLANET_MIN_RADIUS);
        while (!IsPossiblePlanetPosition(point, radius, MINIMUM_PLANET_DISTANCE)) {
            point = RandomPoint();
            radius = PLANET_MIN_RADIUS + Generator() % size_t(PLANET_MAX_RADIUS - PLANET_MIN_RADIUS);
        }
        TPlanet planet;
        planet.Id = Planets.size();
        planet.Position = point;
        planet.PlayerId = -1;
        planet.Energy = PLANET_MIN_ENERGY + Generator() % size_t(PLANET_MAX_ENERGY - PLANET_MIN_ENERGY);
        planet.Radius = radius;
        planet.Type = Generator() % MAX_PLANET_TYPES;
        planet.SpawnCounter = 0;
        Planets[planet.Id] = planet;
    }
}

void TWorld::UpdatePlanetEnergy() {
    for (auto&& p: Planets) {
        TPlanet& planet = p.second;
        if (planet.PlayerId == -1) {
            continue;   // skip neutral planets
        }
        float energy = planet.Radius / 10.0;
        planet.Energy += energy;
    }
}

inline void LimitSpeed(TPointF& speed, float limit = 3.5) {
    float speedAbs = Distance(TPointF(), speed);
    if (speedAbs > limit) {
        speed = (speed / speedAbs) * limit;
    }
}


TPointF TWorld::Rule1(size_t shipNum) {
    TPointF massCenter;
    size_t ships = 0;
    for (int i = 0; i < Ships.size(); ++i) {
        if (i != shipNum &&
            Ships[i].Group == Ships[shipNum].Group)
        {
            massCenter += Ships[i].Position;
            ++ships;
        }
    }
    if (!ships) {
        return TPointF({0, 0});
    }
    massCenter /= ships;
    return (massCenter - Ships[shipNum].Position) / 160;
}

TPointF TWorld::Rule2(size_t shipNum) {
    TPointF c(0, 0);
    for (int i = 0; i < Ships.size(); ++i) {
        if (i != shipNum) {
            float distance = Distance(Ships[i].Position, Ships[shipNum].Position);
            if (distance < 21) {
                TPointF direction = Ships[i].Position - Ships[shipNum].Position;
                Normalize(direction);
                direction *= (33 - distance);
                c -= direction;
            }
        }
    }
    return c / 35;
}

TPointF TWorld::Rule3(size_t shipNum) {
    TPointF massCenter;
    size_t ships = 0;
    for (int i = 0; i < Ships.size(); ++i) {
        if (i != shipNum &&
            Ships[i].Group == Ships[shipNum].Group)
        {
            massCenter += Ships[i].Speed;
            ++ships;
        }
    }
    if (!ships) {
        return TPointF(0, 0);
    }
    massCenter /= ships;
    return (massCenter - Ships[shipNum].Speed) / 18;
}

TPointF TWorld::Rule4(size_t shipNum) {
    TPointF direction = Ships[shipNum].Target - Ships[shipNum].Position;
    LimitSpeed(direction, 0.85);
    return direction;
}

TPointF TWorld::Rule5(size_t shipNum) {
    TPointF c(0, 0);
    for (int i = 0; i < Planets.size(); ++i) {
        if (Ships[shipNum].Target != Planets[i].Position) {
            float distance = Distance(Planets[i].Position, Ships[shipNum].Position);
            if (distance < 10.0 + 1.4 * Planets[i].Radius) {
                TPointF direction = Planets[i].Position - Ships[shipNum].Position;
                Normalize(direction);
                direction *= (18.0 + 1.4 * Planets[i].Radius - distance);
                c -= direction;
            }
        }
    }
    return c / 8;
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

void TWorld::ProcessShips() {
    std::vector<TShip> newShips;
    newShips.reserve(Ships.size());

    for (size_t i = 0; i < (size_t)Ships.size(); ++i) {
        TShip& ship = Ships[i];
        TPointF v1 = Rule1(i);
        TPointF v2 = Rule2(i);
        TPointF v3 = Rule3(i);
        TPointF v4 = Rule4(i);
        TPointF v5 = Rule5(i);
        ship.Speed = ship.Speed + v1 + v2 + v3 + v4 + v5;
        LimitSpeed(ship.Speed);

        ship.Position += ship.Speed;
        if (!ProcessCollision(ship) &&
            ship.Position.X >= 0 && ship.Position.X < WORLD_WIDTH &&
            ship.Position.Y >= 0 && ship.Position.Y < WORLD_HEIGHT)
        {
            newShips.push_back(ship);
        }
    }
    Ships.swap(newShips);
}

void TWorld::SpawnShips(TPlanet& from, TPlanet& to, float energyPercents,
                        size_t playerId, size_t maxShips)
{
    float totalShipsEnergy = from.Energy * energyPercents;
    from.Energy -= totalShipsEnergy;
    size_t shipsCount = std::min(1 + (int)(totalShipsEnergy / 7.0f), int(maxShips));
    float energyPerShip = totalShipsEnergy / shipsCount;
    TPointF direction = to.Position - from.Position;
    Normalize(direction);

    TPointF shipPosition = from.Position;
    shipPosition.X = shipPosition.X + direction.X * (from.Radius + 6.0f);
    shipPosition.Y = shipPosition.Y + direction.Y * (from.Radius + 6.0f);
    TPointF shipSpeed = direction * 2.5f;

    uint16_t group = ++GroupsCounter;

    for (size_t i = 0; i < shipsCount; ++i) {
        TShip ship;
        ship.Position = shipPosition;
        ship.Position.X = ship.Position.X + Generator() % 6 - 3;
        ship.Position.Y = ship.Position.Y + Generator() % 6 - 3;
        ship.Energy = energyPerShip;
        ship.PlayerId = playerId;
        ship.Speed = shipSpeed;
        ship.Target = to.Position;
        ship.Group = group;
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

void TWorld::CheckRoundEnd() {
    std::unordered_set<int> playersLeft;
    for (size_t i = 0; i < (size_t)Planets.size(); ++i) {
        if (Planets[i].PlayerId == -1) {
            continue;
        }
        playersLeft.insert(Planets[i].PlayerId);
    }
    if (playersLeft.size() < 2) {
        DoRestartRound();
    }
}

}
