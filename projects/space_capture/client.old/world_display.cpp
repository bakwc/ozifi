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

    if (World->RoundStartsAt != uint8_t(-1)) {
        DrawRoundRestart(painter, World->RoundStartsAt);
    } else if (World->WaitingPlayers) {
        DrawWaitingPlayers(painter);
    } else {
        for (size_t i = 0; i < World->Planets.size(); ++i) {
            DrawPlanet(painter, World->Planets[i]);
        }
        for (size_t i = 0; i < World->Ships.size(); ++i) {
            DrawShip(painter, World->Ships[i]);
        }
        DrawPower(painter);
        DrawSelection(painter);
    }
}

inline QColor GetQColor(NSpace::EColor color) {
    switch (color) {
        case NSpace::CR_Cyan: return Qt::cyan;
        case NSpace::CR_Blue: return Qt::blue;
        case NSpace::CR_Green: return Qt::green;
        case NSpace::CR_Red: return Qt::red;
        case NSpace::CR_White: return Qt::white;
        case NSpace::CR_Yellow: return Qt::yellow;
    }
    return Qt::gray;
}

void TWorldDisplay::DrawPlanet(QPainter &painter, const NSpace::TPlanet& planet) {

    glLoadIdentity();

    float radius = planet.Radius * World->Scale;
    float x = planet.X * World->Scale + World->OffsetX;
    float y = planet.Y * World->Scale + World->OffsetY;
    QColor planetColor = Qt::gray;
    if (planet.PlayerId != -1) {
        planetColor = GetQColor(World->IdToPlayer[planet.PlayerId]->Color);
    }

    glTranslatef(x, y, 300);
    glScalef(radius, radius, radius);

    glRotatef(Ang, 0, 1, 0);
    glRotatef(15, 1, 0, 0);

    const TPlanetGraphics& planetGraphics = GraphicManager.GetImage(planet.Type, radius * 10, planetColor);

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

    QString energyString = QString("%1").arg(planet.Energy);
    QFontMetrics fm(painter.font());
    int textHeight = fm.height();
    int textWidth = fm.width(energyString);
    if (planet.Energy != -1 &&
       (planet.PlayerId == -1 ||
        planet.PlayerId == World->SelfId))
    {
        const QImage& fontBackground = GraphicManager.GetFontBackground(World->Scale);
        painter.drawImage(x - fontBackground.width() / 2, y - textHeight + textHeight / 3, fontBackground);
        pen.setColor(Qt::white);
        painter.setPen(pen);
        painter.drawText(x - textWidth / 2, y + textHeight / 3, energyString);
    }

    if (planet.PlayerId == World->SelfId &&
        World->SelectedPlanets.find(planet.ID) != World->SelectedPlanets.end())
    {
        pen.setColor(planetColor);
        painter.setPen(pen);
        painter.drawEllipse(x - radius - 4, y - radius - 4, radius * 2 + 8, radius * 2 + 8);
    } else if (World->SelectedTarget.is_initialized() &&
               planet.ID == *World->SelectedTarget)
    {
        NSpace::TPlayer* selfPlayer = World->SelfPlayer();
        if (selfPlayer != nullptr) {
            planetColor = GetQColor(selfPlayer->Color);
            pen.setColor(planetColor);
            painter.setPen(pen);
            painter.drawEllipse(x - radius - 4, y - radius - 4, radius * 2 + 8, radius * 2 + 8);
        }
    }
}

void TWorldDisplay::DrawShip(QPainter& painter, const NSpace::TShip& ship) {
    int x = ship.X * World->Scale + World->OffsetX;
    int y = ship.Y * World->Scale + World->OffsetY;

    QColor shipColor = GetQColor(World->IdToPlayer[ship.PlayerID]->Color);
    QImage shipImage = GraphicManager.GetShip(World->Scale, shipColor);
    QTransform transform;
    transform.rotateRadians(float(ship.Angle) / 100.0 + M_PI_2);
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
