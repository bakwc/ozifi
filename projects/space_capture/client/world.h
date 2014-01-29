#pragma once

#include <QObject>
#include <QHash>

#include <projects/space_capture/lib/space.pb.h>

class TWorld: public QObject, public Space::TWorld {
    Q_OBJECT
public:
    explicit TWorld();
    void UpdateWorld(Space::TWorld world);
signals:
    void OnWorldUpdated();
public:
    float ScaleX;
    float ScaleY;
    QHash<size_t, Space::TPlayer*> IdToPlayer;
    QString PlayerName;
};
