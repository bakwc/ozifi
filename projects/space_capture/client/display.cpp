#include <QFontMetrics>
#include <QRgb>
#include <QDebug>

#include <projects/space_capture/lib/defines.h>

#include "display.h"

TDisplay::TDisplay(QGLWidget* parent)
    : QGLWidget( parent)
    , Frame(WORLD_WIDTH, WORLD_HEIGHT, QImage::Format_ARGB32)
    , CurrentDisplay(nullptr)
    , CurrentControl(nullptr)
{
    QGLFormat f = QGLFormat::defaultFormat();
    f.setSampleBuffers(true);
    QGLFormat::setDefaultFormat(f);

    setGeometry(x(), y(), WORLD_WIDTH, WORLD_HEIGHT);
    setMouseTracking(true);
    setAutoFillBackground(false);
}

TDisplay::~TDisplay() {
}

void TDisplay::mouseReleaseEvent(QMouseEvent* e) {
    if (CurrentControl) {
        CurrentControl->OnMouseEvent(*e, false);
    }
}

void TDisplay::mousePressEvent(QMouseEvent* e) {
    if (CurrentControl) {
        CurrentControl->OnMouseEvent(*e, true);
    }
}

void TDisplay::mouseMoveEvent(QMouseEvent* e) {
    if (CurrentControl) {
        CurrentControl->OnMouseMove(*e);
    }
}

void TDisplay::wheelEvent(QWheelEvent* e) {
    if (CurrentControl) {
     CurrentControl->OnWheelEvent(*e);
    }
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

    if (CurrentControl) {
        CurrentControl->OnResized(e);
    }
}

void TDisplay::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);

    Frame.fill(qRgba(0, 0, 0, 0));
    QPainter painter(&Frame);
    painter.setRenderHint(QPainter::Antialiasing);

    if (CurrentDisplay) {
        CurrentDisplay->Draw(painter);
    }

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

void TDisplay::UpdateContol(IControlable *control) {
    CurrentControl = control;
}

void TDisplay::UpdateDisplay(IDrawable *draw) {
    CurrentDisplay = draw;
}

void TDisplay::Render() {
    updateGL();
}
