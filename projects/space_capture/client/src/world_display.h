#pragma once

#include "display.h"
#include "world.h"

class TApplication;
class TWorldDisplay: public IDrawable, public IControlable {
public:
    TWorldDisplay(TWorld* world, gameplay::Scene* scene, TApplication* app);
    void Draw(float elapsedTime) override;
    void OnResized(size_t width, size_t heigth) override;
private:
    void DrawPlanet(const NSpaceEngine::TPlanet& planet);
    void DrawShip(const NSpaceEngine::TShip& ship);
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
    std::vector<gameplay::Material*> Materials;
    gameplay::SpriteBatch* Ship;
    gameplay::Font* Font;
    gameplay::Font* FontBig;
};
