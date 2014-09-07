#pragma once

#include "display.h"
#include "world.h"
#include "graphic_manager.h"
#include "sphere.h"

class TWorldDisplay: public IDrawable {
public:
    TWorldDisplay(TWorld* world);
    void Draw(QPainter& painter);
private:
    void DrawPlanet(QPainter& painter, const NSpace::TPlanet& planet);
    void DrawShip(QPainter& painter, const NSpace::TShip& ship);
    void DrawSelection(QPainter& painter);
    void DrawPower(QPainter& painter);
    void DrawRoundRestart(QPainter& painter, int restartTime);
    void DrawWaitingPlayers(QPainter& painter);
private:
    TWorld* World;
    TGraphicManager GraphicManager;
    SolidSphere Sphere;
    SolidSphere LittleSphere;
    float Ang;
};
