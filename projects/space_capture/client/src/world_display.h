#pragma once

#include "display.h"
#include "world.h"

class TApplication;
class TWorldDisplay: public IDrawable {
public:
    TWorldDisplay(TWorld* world, gameplay::Scene* scene, TApplication* app);
    void Draw(float elapsedTime) override;
private:
    void DrawPlanet(const NSpace::TPlanet& planet);
    void DrawShip(const NSpace::TShip& ship);
    void DrawSelection();
    void DrawPower();
    void DrawRoundRestart(int restartTime);
    void DrawWaitingPlayers();
private:
    TWorld* World;
    gameplay::Scene* Scene;
    TApplication* Application;
    float Ang;
    gameplay::Node* Sphere;
    gameplay::SpriteBatch* Ship;
    gameplay::Font* Font;
    gameplay::Font* FontBig;
};
