#include <QFontMetrics>
#include <QRgb>
#include <QDebug>

#include <projects/space_capture/lib/defines.h>

#include "display.h"

TDisplay::TDisplay(TWorld* world, QWidget *parent)
    : QWidget(parent)
    , World(world)
    , Frame(WORLD_WIDTH, WORLD_HEIGHT, QImage::Format_RGB32)
{
    setGeometry(x(), y(), WORLD_WIDTH, WORLD_HEIGHT);
    setMouseTracking(true);
}

TDisplay::~TDisplay() {
}

void TDisplay::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.drawImage(0, 0, Frame);
}

void TDisplay::mouseReleaseEvent(QMouseEvent* e) {
    emit OnMouseEvent(*e, false);
}

void TDisplay::mousePressEvent(QMouseEvent* e) {
    emit OnMouseEvent(*e, true);
}

void TDisplay::mouseMoveEvent(QMouseEvent* e) {
    emit OnMouseMove(*e);
}

void TDisplay::resizeEvent(QResizeEvent* e) {
    emit OnResized(*e);
}

void TDisplay::RedrawWorld() {
    Frame = QImage(width(), height(), QImage::Format_RGB32);
    Frame.fill(Qt::black);
    QPainter painter(&Frame);
    painter.setRenderHint(QPainter::Antialiasing);
    for (size_t i = 0; i < World->planets_size(); ++i) {
        DrawPlanet(painter, World->planets(i));
    }
    for (size_t i = 0; i < World->ships_size(); ++i) {
        DrawShip(painter, World->ships(i));
    }
    DrawSelection(painter);
    update();
}

inline QColor GetQColor(Space::EColor color) {
    switch (color) {
        case Space::CR_Cyan: return Qt::cyan;
        case Space::CR_Blue: return Qt::blue;
        case Space::CR_Green: return Qt::green;
        case Space::CR_Red: return Qt::red;
        case Space::CR_White: return Qt::white;
        case Space::CR_Yellow: return Qt::yellow;
    }
    return Qt::gray;
}

void TDisplay::DrawPlanet(QPainter& painter, const Space::TPlanet& planet) {
    int radius = planet.radius() * World->Scale;
    int x = planet.x() * World->Scale + World->OffsetX;
    int y = planet.y() * World->Scale + World->OffsetY;
    QColor planetColor = Qt::gray;
    if (planet.playerid() != -1) {
        planetColor = GetQColor(World->IdToPlayer[planet.playerid()]->color());
    }
    QPen pen(planetColor);
    pen.setWidth(2);
    painter.setPen(pen);
    //painter.drawEllipse(x  - radius, y  - radius, radius * 2, radius * 2);

    const QImage& currentPlanet = GraphicManager.GetImage(planet.type(), radius * 2, planetColor);
    painter.drawImage(x - radius, y - radius, currentPlanet);

    QString energyString = QString("%1").arg(planet.energy());
    QFontMetrics fm = fontMetrics();
    int textHeight = fm.height();
    int textWidth = fm.width(energyString);
    if (planet.energy() != -1 &&
       (planet.playerid() == -1 ||
        planet.playerid() == World->selfid()))
    {
        pen.setColor(Qt::white);
        painter.setPen(pen);
        painter.drawText(x - textWidth / 2, y + textHeight / 3, energyString);
    }

    if (planet.playerid() == World->selfid() &&
        World->SelectedPlanets.find(planet.id()) != World->SelectedPlanets.end())
    {
        pen.setColor(planetColor);
        painter.setPen(pen);
        painter.drawEllipse(x  - radius - 4, y  - radius - 4, radius * 2 + 8, radius * 2 + 8);
    } else if (World->SelectedTarget.is_initialized() &&
               planet.id() == *World->SelectedTarget)
    {
        Space::TPlayer* selfPlayer = World->SelfPlayer();
        if (selfPlayer != nullptr) {
            planetColor = GetQColor(selfPlayer->color());
            pen.setColor(planetColor);
            painter.setPen(pen);
            painter.drawEllipse(x  - radius - 4, y  - radius - 4, radius * 2 + 8, radius * 2 + 8);
        }
    }
}

void TDisplay::DrawShip(QPainter& painter, const Space::TShip& ship) {
    int r = 5 * World->Scale;
    int x = ship.x() * World->Scale - r + World->OffsetX;
    int y = ship.y() * World->Scale - r + World->OffsetY;
    painter.setPen(GetQColor(World->IdToPlayer[ship.playerid()]->color()));
    painter.drawEllipse(x, y, r * 2, r * 2); // todo: draw ship another way
}

void TDisplay::DrawSelection(QPainter& painter) {
    if (World->Selection.is_initialized()) {
        painter.setPen(Qt::cyan);
        int x = std::min(World->Selection->From.x(), World->Selection->To.x());
        int y = std::min(World->Selection->From.y(), World->Selection->To.y());
        int w = abs(World->Selection->From.x() - World->Selection->To.x());
        int h = abs(World->Selection->From.y() - World->Selection->To.y());
        painter.drawRect(x, y, w, h);
    }
}
