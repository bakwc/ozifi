#pragma once

#include <QObject>
#include <QPointF>
#include <vector>
#include <unordered_set>
#include <unordered_map>

#include <boost/optional.hpp>

#include <projects/space_capture/lib/space.h>
#include <projects/space_capture/lib/defines.h>

struct TSelection {
    QPointF From;
    QPointF To;
};

class TWorld: public QObject, public NSpace::TWorld {
    Q_OBJECT
public:
    explicit TWorld();
    void UpdateWorld(NSpace::TWorld world);
    void UpdateSelection(QPointF from, QPointF to);
    void RemoveSelection();
    NSpace::TPlayer* SelfPlayer();
signals:
    void OnWorldUpdated();
public:
    float Scale;
    size_t OffsetX;
    size_t OffsetY;
    std::unordered_map<size_t, NSpace::TPlayer*> IdToPlayer;
    std::unordered_set<size_t> SelectedPlanets;
    boost::optional<size_t> SelectedTarget;
    QString PlayerName;
    boost::optional<TSelection> Selection;
    int Power; // percents
};
