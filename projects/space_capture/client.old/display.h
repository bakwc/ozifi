#pragma once

#include <QtOpenGL/QGLWidget>
#include <QtOpenGL/QGLFramebufferObject>
#include <QImage>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QWheelEvent>
#include <QPainter>

class IDrawable {
public:
    virtual void Draw(QPainter& painter) = 0;
};

class IControlable {
public:
    virtual void OnMouseEvent(QMouseEvent event, bool pressed) { // pressed or release
    }
    virtual void OnMouseMove(QMouseEvent event) {
    }
    virtual void OnResized(QResizeEvent event) {
    }
    virtual void OnWheelEvent(QWheelEvent event) {
    }
};

class TDisplay: public QGLWidget
{
    Q_OBJECT
public:
    TDisplay(QGLWidget* parent = 0);
    ~TDisplay();
    void mouseReleaseEvent(QMouseEvent* e);
    void mousePressEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);
    void wheelEvent(QWheelEvent* e);
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();
    void UpdateContol(IControlable* control = nullptr);
    void UpdateDisplay(IDrawable* draw = nullptr);
public slots:
    void Render();
private:
    IDrawable* CurrentDisplay;
    IControlable* CurrentControl;
    QImage Frame;
};
