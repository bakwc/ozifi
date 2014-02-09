#pragma once

#include <QtOpenGL/QGLWidget>
#include <QtOpenGL/QGLFramebufferObject>
#include <QImage>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QPainter>

#include "world.h"
#include "graphic_manager.h"
#include "sphere.h"

class TDisplay: public QGLWidget
{
    Q_OBJECT
public:
    TDisplay(TWorld* world, QGLWidget *parent = 0);
    ~TDisplay();
    void mouseReleaseEvent(QMouseEvent* e);
    void mousePressEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();
signals:
    void OnMouseEvent(QMouseEvent event, bool pressed); // pressed or release
    void OnMouseMove(QMouseEvent event);
    void OnResized(QResizeEvent event);
public slots:
    void RedrawWorld();
private:
    void DrawPlanet(QPainter& painter, const Space::TPlanet& planet);
    void DrawShip(QPainter& painter, const Space::TShip& ship);
    void DrawSelection(QPainter& painter);
private:
    TWorld* World;
    QImage Frame;
    TGraphicManager GraphicManager;
    SolidSphere Sphere;
    SolidSphere LittleSphere;
    float Ang;
};
