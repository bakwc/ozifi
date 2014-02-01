#pragma once

#include <QWidget>
#include <QImage>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QPainter>

#include "world.h"

class TDisplay: public QWidget
{
    Q_OBJECT
public:
    TDisplay(TWorld* world, QWidget *parent = 0);
    ~TDisplay();
    void paintEvent(QPaintEvent*);
    void mouseReleaseEvent(QMouseEvent* e);
    void mousePressEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);
    void resizeEvent(QResizeEvent* e);
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
};
