#pragma once

#include "display.h"
#include "world.h"

class TApplication;
class TWorldDisplay: public IDrawable, public IControlable {
public:
    TWorldDisplay(TWorld* world, gameplay::Scene* scene, TApplication* app);
    void Draw(float elapsedTime) override;
    void OnResized(size_t width, size_t height) override;
private:
    void DrawPlanet(const NSpaceEngine::TPlanet& planet);
    void DrawShip(const NSpaceEngine::TShip& ship);
    void AddPoint(float x, float y);
    void DrawRect(int x1, int y1, int x2, int y2);
    void DrawCircle(int x, int y, int radius);
    void DrawSelection();
    void DrawPower();
    void DrawScore();
    void DrawSingleScore(uint8_t playerId, size_t score, size_t position);
    void DrawRoundRestart(int restartTime);
    void DrawWaitingPlayers();
private:
    TWorld* World;
    gameplay::Scene* Scene;
    TApplication* Application;
    float Ang;
    gameplay::Node* Sphere;
    gameplay::Model* Circle;
    gameplay::Model* Selection;
    std::vector<gameplay::Material*> Materials;
    gameplay::SpriteBatch* Ship;
    gameplay::Font* Font;
    gameplay::Font* FontBig;
    std::vector<float> LinePosition;
    gameplay::Node* LineNode;
    size_t Width;
    size_t Height;
};
