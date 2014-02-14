#include <QDebug>

#include "main_menu.h"

void TMenuElement::Draw(QPainter& painter, const QSize& displaySize) {
    int x = Position.x() * displaySize.width();
    int y = Position.y() * displaySize.height();
    QString text = Text;
    if (Selected) {
        text = "- " + text + " -";
    }
    QFontMetrics fm(painter.font());
    int textHeight = fm.height();
    int textWidth = fm.width(text);
    painter.drawText(x - textWidth / 2, y - textHeight / 2, text);
}

void TMenuElement::CheckSelection(const QPoint& mousePosition, const QSize& displaySize) {
    int x = Position.x() * displaySize.width();
    int y = Position.y() * displaySize.height();
    QFontMetrics fm(QFont("arial", 18));
    int textHeight = fm.height();
    int textWidth = fm.width(Text);
    y -= textHeight;    // todo: why this hack?
    if (mousePosition.x() >= x - textWidth / 2 && mousePosition.x() <= x + textWidth / 2 &&
        mousePosition.y() >= y - textHeight / 2 && mousePosition.y() <= y + textHeight / 2)
    {
        Selected = true;
    } else {
        Selected = false;
    }
}

void TMenuElement::CheckClicked(const QPoint& mousePosition, const QSize& displaySize) {
    Q_UNUSED(mousePosition);
    if (Selected) {
        emit Clicked();
    }
}

TMainMenu::TMainMenu(QObject *parent) :
    QObject(parent)
{
    Elements.push_back(std::make_shared<TMenuElement>(QPointF(0.5, 0.45), "Quick Game"));
    connect(Elements.back().get(), &TMenuElement::Clicked, this, &TMainMenu::QuickGame);

    Elements.push_back(std::make_shared<TMenuElement>(QPointF(0.5, 0.55), "Exit"));
    connect(Elements.back().get(), &TMenuElement::Clicked, this, &TMainMenu::Exit);
    startTimer(30);
}

void TMainMenu::Draw(QPainter& painter) {
    painter.setPen(Qt::white);
    painter.setFont(QFont("arial", 18));
    for (auto& element: Elements) {
        element->Draw(painter, DisplaySize);
    }
}

void TMainMenu::OnResized(QResizeEvent event) {
    DisplaySize = event.size();
}

void TMainMenu::OnMouseMove(QMouseEvent event) {
    for (auto& element: Elements) {
        element->CheckSelection(event.pos(), DisplaySize);
    }
}

void TMainMenu::OnMouseEvent(QMouseEvent event, bool pressed) {
    if (!pressed) {
        for (auto& element: Elements) {
            element->CheckClicked(event.pos(), DisplaySize);
        }
    }
}

void TMainMenu::timerEvent(QTimerEvent*) {
    emit Render();
}
