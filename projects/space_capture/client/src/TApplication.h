#pragma once

#include "gameplay.h"

class TApplication: public gameplay::Game {
public:
    TApplication();
    void keyEvent(gameplay::Keyboard::KeyEvent evt, int key);
    void touchEvent(gameplay::Touch::TouchEvent evt, int x, int y, unsigned int contactIndex);
protected:
    void initialize();
    void finalize();
    void update(float elapsedTime);
    void render(float elapsedTime);
private:
    bool drawScene(gameplay::Node* node);
    void initializeMaterial(gameplay::Material* material, bool lighting, bool specular);
private:
    gameplay::Scene* Scene;
    bool Wireframe;
};
