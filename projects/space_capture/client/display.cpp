#include <QFontMetrics>
#include <QRgb>
#include <QDebug>

#include <projects/space_capture/lib/defines.h>

#include "display.h"

TDisplay::TDisplay(TWorld* world, QGLWidget *parent)
    : QGLWidget( parent)
    , World(world)
    , Frame(WORLD_WIDTH, WORLD_HEIGHT, QImage::Format_ARGB32)
    , Sphere(1.0, 12, 24)
    , LittleSphere(1.0, 6, 12)
    , Ang(0)
{
    setGeometry(x(), y(), WORLD_WIDTH, WORLD_HEIGHT);
    setMouseTracking(true);
    setAutoFillBackground(false);
}

TDisplay::~TDisplay() {
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

void TDisplay::wheelEvent(QWheelEvent* e) {
    emit OnWheelEvent(*e);
}

void TDisplay::initializeGL() {

    glEnable(GL_MULTISAMPLE);
    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHTING);
    glEnable(GL_NORMALIZE);

    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    GLfloat lightAmbient[]= { 0.8f, 0.8f, 0.8f, 1.0f };
    glLightfv(GL_LIGHT1, GL_AMBIENT, lightAmbient);
    GLfloat lightDiffuse[]= { 1.0f, 1.0f, 1.0f, 1.0f };
    glLightfv(GL_LIGHT1, GL_DIFFUSE, lightDiffuse);
    GLfloat lightPosition[]= { 0.7f, -0.5f, 1.0f, 0.0f };
    glLightfv(GL_LIGHT1, GL_POSITION, lightPosition);
}

void TDisplay::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, w, h, 0, -800, 800);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    QResizeEvent e(QSize(w, h), QSize());

    Frame = QImage(w, h, QImage::Format_ARGB32);

    emit OnResized(e);
}

void TDisplay::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);

    Frame.fill(qRgba(0, 0, 0, 0));
    QPainter painter(&Frame);
    painter.setRenderHint(QPainter::Antialiasing);

    for (size_t i = 0; i < World->planets_size(); ++i) {
        DrawPlanet(painter, World->planets(i));
    }

    for (size_t i = 0; i < World->ships_size(); ++i) {
        DrawShip(painter, World->ships(i));
    }

    DrawPower(painter);
    DrawSelection(painter);

    glMatrixMode(GL_MODELVIEW);

    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);

    QPainter rawPainter(this);
    rawPainter.setRenderHint(QPainter::Antialiasing);
    rawPainter.drawImage(0, 0, Frame);
}

void TDisplay::RedrawWorld() {
    Ang += 1.0;
    updateGL();
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

void TDisplay::DrawPlanet(QPainter &painter, const Space::TPlanet& planet) {

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
    QFontMetrics fm = fontMetrics();
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

void TDisplay::DrawShip(QPainter& painter, const Space::TShip& ship) {
    int x = ship.x() * World->Scale + World->OffsetX;
    int y = ship.y() * World->Scale + World->OffsetY;

    QColor shipColor = GetQColor(World->IdToPlayer[ship.playerid()]->color());
    QImage shipImage = GraphicManager.GetShip(World->Scale, shipColor);
    QTransform transform;
    transform.rotateRadians(float(ship.angle()) / 100.0 + M_PI_2);
    shipImage = shipImage.transformed(transform);
    painter.drawImage(x - shipImage.width() / 2, y - shipImage.height() / 2, shipImage);
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

void TDisplay::DrawPower(QPainter& painter) {
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
