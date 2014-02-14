#include <QDebug>

#include "world_display.h"

TWorldDisplay::TWorldDisplay(TWorld *world)
    : World(world)
    , Sphere(1.0, 12, 24)
    , LittleSphere(1.0, 6, 12)
    , Ang(0)
{
}

void TWorldDisplay::Draw(QPainter& painter) {
    Ang += 1.0;

    if (World->has_roundstartsat()) {
        DrawRoundRestart(painter, World->roundstartsat());
    } else if (World->has_waitingplayers()) {
        DrawWaitingPlayers(painter);
    } else {
        for (size_t i = 0; i < World->planets_size(); ++i) {
            DrawPlanet(painter, World->planets(i));
        }
        for (size_t i = 0; i < World->ships_size(); ++i) {
            DrawShip(painter, World->ships(i));
        }
        DrawPower(painter);
        DrawSelection(painter);
    }
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

void TWorldDisplay::DrawPlanet(QPainter &painter, const Space::TPlanet& planet) {

    glLoadIdentity();

    float radius = planet.radius() * World->Scale;
    float x = planet.x() * World->Scale + World->OffsetX;
    float y = planet.y() * World->Scale + World->OffsetY;
    QColor planetColor = Qt::gray;
    if (planet.playerid() != -1) {
        planetColor = GetQColor(World->IdToPlayer[planet.playerid()]->color());
    }

    glTranslatef(x, y, 300);
    glScalef(radius, radius, radius);

    glRotatef(Ang, 0, 1, 0);
    glRotatef(15, 1, 0, 0);

    const TPlanetGraphics& planetGraphics = GraphicManager.GetImage(planet.type(), radius * 10, planetColor);

    glBindTexture(GL_TEXTURE_2D, planetGraphics.TextureId);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glEnable(GL_TEXTURE_2D);
    if (radius > 20) {
        Sphere.draw();
    } else {
        LittleSphere.draw();
    }
    glDisable(GL_TEXTURE_2D);


    QPen pen(planetColor);
    pen.setWidth(2);

    QString energyString = QString("%1").arg(planet.energy());
    QFontMetrics fm(painter.font());
    int textHeight = fm.height();
    int textWidth = fm.width(energyString);
    if (planet.energy() != -1 &&
       (planet.playerid() == -1 ||
        planet.playerid() == World->selfid()))
    {
        const QImage& fontBackground = GraphicManager.GetFontBackground(World->Scale);
        painter.drawImage(x - fontBackground.width() / 2, y - textHeight + textHeight / 3, fontBackground);
        pen.setColor(Qt::white);
        painter.setPen(pen);
        painter.drawText(x - textWidth / 2, y + textHeight / 3, energyString);
    }

    if (planet.playerid() == World->selfid() &&
        World->SelectedPlanets.find(planet.id()) != World->SelectedPlanets.end())
    {
        pen.setColor(planetColor);
        painter.setPen(pen);
        painter.drawEllipse(x - radius - 4, y - radius - 4, radius * 2 + 8, radius * 2 + 8);
    } else if (World->SelectedTarget.is_initialized() &&
               planet.id() == *World->SelectedTarget)
    {
        Space::TPlayer* selfPlayer = World->SelfPlayer();
        if (selfPlayer != nullptr) {
            planetColor = GetQColor(selfPlayer->color());
            pen.setColor(planetColor);
            painter.setPen(pen);
            painter.drawEllipse(x - radius - 4, y - radius - 4, radius * 2 + 8, radius * 2 + 8);
        }
    }
}

void TWorldDisplay::DrawShip(QPainter& painter, const Space::TShip& ship) {
    int x = ship.x() * World->Scale + World->OffsetX;
    int y = ship.y() * World->Scale + World->OffsetY;

    QColor shipColor = GetQColor(World->IdToPlayer[ship.playerid()]->color());
    QImage shipImage = GraphicManager.GetShip(World->Scale, shipColor);
    QTransform transform;
    transform.rotateRadians(float(ship.angle()) / 100.0 + M_PI_2);
    shipImage = shipImage.transformed(transform);
    painter.drawImage(x - shipImage.width() / 2, y - shipImage.height() / 2, shipImage);
}

void TWorldDisplay::DrawSelection(QPainter& painter) {
    if (World->Selection.is_initialized()) {
        painter.setPen(Qt::cyan);
        int x = std::min(World->Selection->From.x(), World->Selection->To.x());
        int y = std::min(World->Selection->From.y(), World->Selection->To.y());
        int w = abs(World->Selection->From.x() - World->Selection->To.x());
        int h = abs(World->Selection->From.y() - World->Selection->To.y());
        painter.drawRect(x, y, w, h);
    }
}

void TWorldDisplay::DrawPower(QPainter& painter) {
    int x = 0.97 * WORLD_WIDTH * World->Scale + World->OffsetX;
    int y = 0.78 * WORLD_HEIGHT * World->Scale + World->OffsetY;
    int width = 0.012 * WORLD_WIDTH * World->Scale;
    int height = 0.2 * WORLD_HEIGHT * World->Scale;
    QPen pen(Qt::white);
    pen.setWidth(2);
    painter.setPen(pen);
    painter.drawRect(x, y, width, height);

    int filled = 0.01 * World->Power * height;
    painter.fillRect(x, y + height - filled, width, filled, Qt::white);
}

void TWorldDisplay::DrawRoundRestart(QPainter& painter, int restartTime) {
    int x = 0.5 * WORLD_WIDTH * World->Scale + World->OffsetX;
    int y = 0.5 * WORLD_HEIGHT * World->Scale + World->OffsetY;
    QPen pen(Qt::white);
    painter.setPen(pen);
    painter.setFont(QFont("arial", 28));
    painter.drawText(x, y, QString("%1").arg(restartTime));
}

void TWorldDisplay::DrawWaitingPlayers(QPainter& painter) {
    int x = 0.5 * WORLD_WIDTH * World->Scale + World->OffsetX;
    int y = 0.5 * WORLD_HEIGHT * World->Scale + World->OffsetY;
    QPen pen(Qt::white);
    painter.setPen(pen);
    painter.setFont(QFont("arial", 28));
    QString text = "Waiting for players...";
    QFontMetrics fm(painter.font());
    int textWidth = fm.width(text);
    painter.drawText(x - textWidth / 2, y, text);
}
