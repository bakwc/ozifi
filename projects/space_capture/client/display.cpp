#include <QFontMetrics>
#include <QRgb>
#include <QDesktopWidget>
#include <QDebug>

#include <projects/space_capture/lib/defines.h>

#include "display.h"

TDisplay::TDisplay(QGLWidget* parent)
    : QGLWidget( parent)
    , CurrentDisplay(nullptr)
    , CurrentControl(nullptr)
{
    size_t requiredWidth = WORLD_WIDTH;
    size_t requiredHeight = WORLD_HEIGHT;

    QDesktopWidget widget;
    if (widget.width() < requiredWidth) {
        requiredWidth = widget.width() * 0.9;
    }
    if (widget.height() < requiredHeight) {
        requiredHeight = widget.height() * 0.9;
    }

    Frame = QImage(requiredWidth, requiredHeight, QImage::Format_ARGB32);

    setGeometry(widget.width() / 2 - requiredWidth / 2,
                widget.height() / 2 - requiredHeight / 2,
                requiredWidth, requiredHeight);
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

    QGLFormat format;
    format.setDoubleBuffer(false);
    format.setSampleBuffers(true);
    setFormat(format);

    glEnable(GL_MULTISAMPLE);
    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHTING);
    glEnable(GL_NORMALIZE);

    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
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

    GLfloat lightAmbient[]= { 0.8f, 0.8f, 0.8f, 1.0f };
    glLightfv(GL_LIGHT1, GL_AMBIENT, lightAmbient);
    GLfloat lightDiffuse[]= { 1.5f, 1.5f, 1.5f, 1.0f };
    glLightfv(GL_LIGHT1, GL_DIFFUSE, lightDiffuse);
    GLfloat lightPosition[]= { 400.7f, -600.5f, 1100.0f, 0.0f };
    glLightfv(GL_LIGHT1, GL_POSITION, lightPosition);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT1);
    glEnable(GL_NORMALIZE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glEnable(GL_MULTISAMPLE);

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
    CurrentControl->OnResized(QResizeEvent(size(), QSize()));
}

void TDisplay::UpdateDisplay(IDrawable *draw) {
    CurrentDisplay = draw;
}

void TDisplay::Render() {
    updateGL();
}
