#include <QDebug>

#include "main_menu.h"

void TMenuElement::Draw(QPainter& painter, const QSize& displaySize) {
    int x = Position.x() * displaySize.width();
    int y = Position.y() * displaySize.height();
    QFontMetrics fm(painter.font());
    int textHeight = fm.height();
    int textWidth = fm.width(Text);
    painter.drawText(x - textWidth / 2, y - textHeight / 2, Text);
}

void TMenuElement::CheckSelection(const QPoint& mousePosition, const QSize& displaySize) {
}

void TMenuElement::CheckClicked(const QPoint& mousePosition, const QSize& displaySize) {
}

TMainMenu::TMainMenu(QObject *parent) :
    QObject(parent)
{
    Elements.push_back(std::make_shared<TMenuElement>(QPointF(0.5, 0.35), "Singleplayer"));
    Elements.push_back(std::make_shared<TMenuElement>(QPointF(0.5, 0.5), "Multiplayer Quick"));
    Elements.push_back(std::make_shared<TMenuElement>(QPointF(0.5, 0.65), "Custom Server"));
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
