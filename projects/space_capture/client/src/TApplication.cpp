#include "TApplication.h"

using namespace gameplay;

TApplication game;

TApplication::TApplication()
    : Scene(NULL)
    , World([this] (const std::string& command) {
        if (State == AS_GameOnline) {
            Network->SendCommand(command);
        }
    })
{
}

void TApplication::initialize() {
    srand(time(NULL));
    World.SetSeed(rand() % 4096);
    Scene = gameplay::Scene::create();
    Scene->setAmbientColor(1.f, 1.f, 1.f);

    Camera* cam = gameplay::Camera::createPerspective(28, float(this->getWidth()) / float(this->getHeight()), 0.25, 10);
    CamNode = Scene->addNode("cam");
    CamNode->setCamera(cam);
    Scene->setActiveCamera(cam);

    WorldDisplay.reset(new TWorldDisplay(&World, Scene, this));
    Control.reset(new TControl(&World, [this](const std::vector<uint8_t>& planetsFrom,
                                              uint8_t planetTo,
                                              uint8_t energyPercent)
    {
        std::lock_guard<std::recursive_mutex> guard(Lock);
        World.Attack(World.SelfId, planetsFrom, planetTo, energyPercent);
    }, this));

    MainMenu.reset(new TMainMenu([this] {
        exit();
    }, [this] {
        Network.reset(new TNetwork("172.28.0.100", 8883, [this](size_t selfId, const std::string& world) {
            std::lock_guard<std::recursive_mutex> guard(Lock);
            World.SelfId = selfId;
            World.SetServer(false);
            World.UpdateWorld(world);

        }, [this] (const std::string& command) {
            std::lock_guard<std::recursive_mutex> guard(Lock);
            World.OnCommandReceived(command);
        }));

        State = AS_GameOnline;
        resizeEvent(getWidth(), getHeight());
    }, [this](size_t level) {
        std::lock_guard<std::recursive_mutex> guard(Lock);
        World.BotLevel = level;
        State = AS_GameSingle;
        World.SetServer(true);
        World.SelfId = rand() % MAX_PLAYERS;
        World.BotId = rand() % MAX_PLAYERS;
        while (World.BotId == World.SelfId) {
            World.BotId = rand() % MAX_PLAYERS;
        }
        World.OnNewPlayer(World.SelfId);
        World.OnNewPlayer(World.BotId);
        GameStart = std::chrono::system_clock::now();
        resizeEvent(getWidth(), getHeight());
    }));

    State = AS_MainMenu;

    resizeEvent(getWidth(), getHeight());
}


void TApplication::finalize() {
    SAFE_RELEASE(Scene);
}

void TApplication::update(float elapsedTime) {
    if (State == AS_GameSingle) {
        std::lock_guard<std::recursive_mutex> guard(Lock);
        auto now = std::chrono::system_clock::now();
        size_t elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - GameStart).count();
        if (elapsed / 50 > Counter) {
            ++Counter;
            size_t botAction = 10;
            if (World.BotLevel == 0) {
                botAction = 220;
            } else if (World.BotLevel == 1) {
                botAction = 10;
            }
            if (Counter % botAction == 0) {
                ProcessBot();
            }
            World.Process();
        }
    }
}

void TApplication::render(float elapsedTime) {
    clear(CLEAR_COLOR_DEPTH, Vector4::zero(), 1.0f, 0);
    std::lock_guard<std::recursive_mutex> guard(Lock);
    if (State == AS_GameOnline || State == AS_GameSingle) {
        WorldDisplay->Draw(elapsedTime);
    } else if (State == AS_MainMenu) {
        MainMenu->Draw(elapsedTime);
    }
}

void TApplication::resizeEvent(unsigned int width, unsigned int height) {
    std::lock_guard<std::recursive_mutex> guard(Lock);
    World.ScreenWidth = width;
    World.ScreenHeight = height;
    this->setViewport(Rectangle(0, 0, width, height));
    CamNode->getCamera()->setAspectRatio(float(width) / height);
    World.Ratio = CamNode->getCamera()->getAspectRatio();
    CamNode->setTranslation(0, 0, 0);
    if (State == AS_GameOnline || State == AS_GameSingle) {
        Control->OnResized(width, height);
        WorldDisplay->OnResized(width, height);
    } else if (State == AS_MainMenu) {
        MainMenu->OnResized(width, height);
    }
}

bool TApplication::mouseEvent(Mouse::MouseEvent evt, int x, int y, int wheelDelta) {
    std::lock_guard<std::recursive_mutex> guard(Lock);
    if (evt == Mouse::MOUSE_MOVE) {
        touchEvent(Touch::TOUCH_MOVE, x, y, 0);
        return true;
    } else if (evt == Mouse::MOUSE_WHEEL) {
        if (State == AS_GameOnline || State == AS_GameSingle) {
            Control->OnWheelEvent(wheelDelta);
            return true;
        }
    }
    return false;
}

void TApplication::ProcessBot() {
    float myEnergy = 0;
    float enemyEnergy = 0;
    std::vector<uint8_t> myPlanets;
    std::vector<uint8_t> enemyPlanets;
    for (auto&& planet: World.Planets) {
        if (planet.second.PlayerId == World.BotId) {
            myPlanets.push_back(planet.second.Id);
            myEnergy += planet.second.Energy;
        } else if (planet.second.PlayerId == World.SelfId) {
            enemyPlanets.push_back(planet.second.Id);
            enemyEnergy += planet.second.Energy;
        }
    }

    std::unordered_map<uint8_t, float> attackDanger;
    std::unordered_map<uint8_t, float> supportDanger;
    for (auto&& ship: World.Ships) {
        if (ship.PlayerId != World.BotId && World.Planets[ship.Target].PlayerId != size_t(-1)) {
            attackDanger[ship.Target] += ship.Energy;
        } else {
            supportDanger[ship.Target] += ship.Energy;
        }
    }

    bool haveDanger = false;
    for (auto&& danger: attackDanger) {
        std::vector<NSpaceEngine::TPlanet*> supportPlanets;
        NSpaceEngine::TPlanet* from = &World.Planets[danger.first];
        if (danger.second < from->Energy ||
            1.1 * danger.second < from->Energy + supportDanger[danger.first])
        {
            continue; // enemy have not enought resources to capture the planet
        }
        haveDanger = true;

        for (auto&& planet: World.Planets) {
            if (planet.second.PlayerId == World.BotId &&
                planet.second.Id != from->Id)
            {
                supportPlanets.push_back(&planet.second);
            }
        }

        std::sort(supportPlanets.begin(), supportPlanets.end(),
                  [from](const NSpaceEngine::TPlanet* first,
                  const NSpaceEngine::TPlanet* second) {
            float r1 = pow(from->Position.X - first->Position.X, 2) + pow(from->Position.Y - first->Position.Y, 2);
            float r2 = pow(from->Position.X - second->Position.X, 2) + pow(from->Position.Y - second->Position.Y, 2);
            return r1 < r2;
        });
        float totalEnergy = from->Energy + supportDanger[danger.first];
        for (auto&& support: supportPlanets) {
            if (support->Energy < 4.f) {
                continue; // do not have enough energy to support
            }
            float requiredEnergy = danger.second - totalEnergy;
            requiredEnergy = std::min(requiredEnergy, support->Energy);
            World.Attack(World.BotId, {support->Id}, from->Id, 100.f * requiredEnergy / support->Energy);
            totalEnergy += requiredEnergy;
            if (totalEnergy > 1.1 * danger.second) {
                break;
            }
        }
    }

    if (!haveDanger) {
        if (enemyEnergy * 1.2 < myEnergy && enemyPlanets.size() == 1) {
            World.Attack(World.BotId, myPlanets, enemyPlanets[0], 100);
            return;
        }
    }

    for (auto&& planet: World.Planets) {
        if (planet.second.PlayerId == World.BotId) {
            // todo: check if can safely leave
            size_t bestPlanet = -1;
            for (auto&& planetToAttack: World.Planets) {
                if (planetToAttack.second.PlayerId != World.BotId) {
                    if (bestPlanet == size_t(-1) ||
                        planetToAttack.second.BetterToAttack(World.Planets[bestPlanet], planet.second))
                    {
                        bestPlanet = planetToAttack.second.Id;
                    }
                }
            }
            if (bestPlanet != size_t(-1) &&
                World.Planets[bestPlanet].Energy < 0.5 * planet.second.Energy &&
                attackDanger[planet.second.Id] < 0.5 * planet.second.Energy)
            {
                World.Attack(World.BotId, {planet.second.Id}, bestPlanet, 50);
            }
        }
    }
}

void TApplication::keyEvent(Keyboard::KeyEvent evt, int key) {
    if (evt == Keyboard::KEY_PRESS) {
        switch (key)
        {
        case Keyboard::KEY_ESCAPE:
            exit();
            break;
        }
    }
}

void TApplication::touchEvent(Touch::TouchEvent evt, int x, int y, unsigned int contactIndex) {
    std::lock_guard<std::recursive_mutex> guard(Lock);
    if (State == AS_GameOnline || State == AS_GameSingle) {
        Control->OnTouchEvent(evt, x, y, contactIndex);
    } else if (State == AS_MainMenu) {
        MainMenu->OnTouchEvent(evt, x, y, contactIndex);
    }
}

void TApplication::project(Vector3 pos, float& x, float& y, bool fixed) {
    CamNode->getCamera()->project(this->getViewport(), pos, &x, &y);
}
