#pragma once

#include <unordered_map>
#include <mutex>

#include "gameplay.h"
#include "world.h"
#include "display.h"
#include "world_display.h"
#include "control.h"
#include "network.h"
#include "main_menu.h"

enum EApplicationState {
    AS_None,
    AS_MainMenu,
    AS_Game
};

class TApplication: public gameplay::Game {
public:
    TApplication();
    void keyEvent(gameplay::Keyboard::KeyEvent evt, int key) override;
    void touchEvent(gameplay::Touch::TouchEvent evt, int x, int y, unsigned int contactIndex) override;
    void project(gameplay::Vector3 pos, float& x, float& y);
protected:
    void initialize() override;
    void finalize() override;
    void update(float elapsedTime) override;
    void render(float elapsedTime) override;
    void resizeEvent(unsigned int width, unsigned int height) override;
    bool mouseEvent(gameplay::Mouse::MouseEvent evt, int x, int y, int wheelDelta) override;
private:
    void initializeMaterial(gameplay::Material* material, bool lighting, bool specular);
private:
    gameplay::Scene* Scene;
    gameplay::Node* CamNode;
    TWorld World;
    std::unique_ptr<TWorldDisplay> WorldDisplay;
    std::unique_ptr<TControl> Control;
    std::unique_ptr<TMainMenu> MainMenu;
    std::unique_ptr<TNetwork> Network;
    uint16_t Counter = 0;
    std::recursive_mutex Lock;
    EApplicationState State = AS_None;
};
