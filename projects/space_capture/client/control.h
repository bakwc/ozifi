#pragma once

#include <QObject>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QWheelEvent>
#include <QTime>
#include <QPoint>

#include "world.h"

enum EControlState {
    CS_None,
    CS_PlanetSelection,
    CS_TargetSelection
};

class TControl: public QObject {
    Q_OBJECT
public:
    explicit TControl(TWorld* world);
public slots:
    void OnMouseEvent(QMouseEvent event, bool mouseDown);
    void OnMouseMove(QMouseEvent event);
    void OnResizeEvent(QResizeEvent event);
    void OnWheelEvent(QWheelEvent event);
signals:
    void OnControl(Space::TControl control);
private:
    void timerEvent(QTimerEvent *);
    void CheckSelection(QPoint from, QPoint to);
    void CheckTargetSelection(QPoint position);
    void SpawnShips();
private:
    TWorld* World;
    QTime LastSendControl;
    EControlState State;
    QPoint SelectionFrom;
};
