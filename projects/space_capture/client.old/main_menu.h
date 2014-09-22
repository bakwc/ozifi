#pragma once

#include <memory>

#include <QObject>
#include <QPainter>

#include "display.h"

class TMenuElement: public QObject {
    Q_OBJECT
public:
    explicit TMenuElement(const QPointF& position, const QString& text)
        : Position(position)
        , Text(text)
        , Selected(false)
    {
    }
    void Draw(QPainter& painter, const QSize& displaySize);
    void CheckSelection(const QPoint& mousePosition, const QSize& displaySize);
    void CheckClicked(const QPoint& mousePosition, const QSize& displaySize);
signals:
    void Clicked();
private:
    QPointF Position;
    QString Text;
    bool Selected;
};

typedef std::shared_ptr<TMenuElement> TMenuElementRef;

class TMainMenu : public QObject, public IDrawable, public IControlable {
    Q_OBJECT
public:
    explicit TMainMenu(QObject *parent = 0);
    void Draw(QPainter& painter);
    void OnResized(QResizeEvent event);
    void OnMouseMove(QMouseEvent event);
    void OnMouseEvent(QMouseEvent event, bool pressed);
signals:
    void Render();
    void Exit();
    void QuickGame();
private:
    void timerEvent(QTimerEvent*);
private:
    std::vector<TMenuElementRef> Elements;
    QSize DisplaySize;
};
