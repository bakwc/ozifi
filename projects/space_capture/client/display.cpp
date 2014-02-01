#include <QFontMetrics>

#include <projects/space_capture/lib/defines.h>

#include "display.h"

TDisplay::TDisplay(TWorld* world, QWidget *parent)
    : QWidget(parent)
    , World(world)
    , Frame(WORLD_WIDTH, WORLD_HEIGHT, QImage::Format_RGB32)
{
    setGeometry(x(), y(), WORLD_WIDTH, WORLD_HEIGHT);
}

TDisplay::~TDisplay() {
}

void TDisplay::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.drawImage(0, 0, Frame);
}

void TDisplay::mouseReleaseEvent(QMouseEvent* e) {
    emit OnMouseEvent(*e, true);
}

void TDisplay::mousePressEvent(QMouseEvent* e) {
    emit OnMouseEvent(*e, false);
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
    for (size_t i = 0; i < World->planets_size(); ++i) {
        DrawPlanet(painter, World->planets(i));
    }
    for (size_t i = 0; i < World->ships_size(); ++i) {
        DrawShip(painter, World->ships(i));
    }
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
    painter.setPen(planetColor);
    painter.drawEllipse(x  - radius, y  - radius, radius * 2, radius * 2);

    QString energyString = QString("%1").arg(planet.energy());
    QFontMetrics fm = fontMetrics();
    int textHeight = fm.height();
    int textWidth = fm.width(energyString);
    if (planet.energy() != -1) {
        painter.drawText(x - textWidth / 2, y + textHeight / 3, energyString);
    }
}

void TDisplay::DrawShip(QPainter& painter, const Space::TShip& ship) {
    int r = 5 * World->Scale;
    int x = ship.x() * World->Scale - r + World->OffsetX;
    int y = ship.y() * World->Scale - r + World->OffsetY;
    painter.setPen(GetQColor(World->IdToPlayer[ship.playerid()]->color()));
    painter.drawEllipse(x, y, r * 2, r * 2); // todo: draw ship another way
}
