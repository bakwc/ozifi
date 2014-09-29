#include "TApplication.h"

using namespace gameplay;

TApplication game;

TApplication::TApplication()
    : Scene(NULL)
    , World([this] (const std::string& command) {
        Network->SendCommand(command);
    })
{
}

void TApplication::initialize() {
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
            World.UpdateWorld(world);

        }, [this] (const std::string& command) {
            std::lock_guard<std::recursive_mutex> guard(Lock);
            World.OnCommandReceived(command);
        }));

        State = AS_Game;
        resizeEvent(getWidth(), getHeight());
    }));

    State = AS_MainMenu;

    resizeEvent(getWidth(), getHeight());
}


void TApplication::finalize() {
    SAFE_RELEASE(Scene);
}

void TApplication::update(float elapsedTime) {
}

void TApplication::render(float elapsedTime) {
    clear(CLEAR_COLOR_DEPTH, Vector4::zero(), 1.0f, 0);
    std::lock_guard<std::recursive_mutex> guard(Lock);
    if (State == AS_Game) {
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
    if (State == AS_Game) {
        Control->OnResized(width, height);
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
        if (State == AS_Game) {
            Control->OnWheelEvent(wheelDelta);
            return true;
        }
    }
    return false;
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
    if (State == AS_Game) {
        Control->OnTouchEvent(evt, x, y, contactIndex);
    } else if (State == AS_MainMenu) {
        MainMenu->OnTouchEvent(evt, x, y, contactIndex);
    }
}

void TApplication::project(Vector3 pos, float& x, float& y) {
    CamNode->getCamera()->project(this->getViewport(), pos, &x, &y);
}
