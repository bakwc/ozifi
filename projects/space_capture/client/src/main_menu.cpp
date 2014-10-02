#include "main_menu.h"

using namespace gameplay;

void TMenuElement::Draw(const TSize& displaySize) {
    int x = Position.X * displaySize.Width;
    int y = Position.Y * displaySize.Height;
    std::string text = Text;
    if (Selected) {
        text = "- " + text + " -";
    }

    unsigned int textWidth, textHeight;
    Font->start();
    Font->measureText(text.c_str(), Font->getSize(), &textWidth, &textHeight);
    x -= 0.5 * textWidth;
    y -= 0.5 * textHeight;
    Vector4 fontColor(1, 1, 1, 1);
    Font->drawText(text.c_str(), x, y, fontColor, Font->getSize());
    Font->finish();
}

void TMenuElement::CheckSelection(const TPoint& mousePosition, const TSize& displaySize) {
    int x = Position.X * displaySize.Width;
    int y = Position.Y * displaySize.Height;

    unsigned int textWidth, textHeight;
    Font->measureText(Text.c_str(), Font->getSize(), &textWidth, &textHeight);

    x -= 0.5 * textWidth;
    y -= 0.5 * textHeight;

    if (mousePosition.X >= x && mousePosition.X <= x + textWidth &&
        mousePosition.Y >= y && mousePosition.Y <= y + textHeight)
    {
        Selected = true;
    } else {
        Selected = false;
    }
}

void TMenuElement::CheckClicked() {
    if (Selected) {
        OnClick();
    }
}

TMainMenu::TMainMenu(std::function<void()> onExit,
                     std::function<void()> onQuickGame,
                     std::function<void()> onSingleGame)
    : OnExit(onExit)
    , OnQuickGame(onQuickGame)
    , OnSingleGame(onSingleGame)
{
    Elements.push_back(std::make_shared<TMenuElement>(TPointF(0.5, 0.4), "Play Single", [this] {
        OnSingleGame();
    }));

    Elements.push_back(std::make_shared<TMenuElement>(TPointF(0.5, 0.5), "Play Online", [this] {
        OnQuickGame();
    }));

    Elements.push_back(std::make_shared<TMenuElement>(TPointF(0.5, 0.6), "Exit", [this] {
        OnExit();
    }));
}

void TMainMenu::Draw(float) {
    for (auto& element: Elements) {
        element->Draw(DisplaySize);
    }
}

void TMainMenu::OnTouchEvent(gameplay::Touch::TouchEvent evt,
                             int x, int y, unsigned int contactIndex)
{
    for (auto& element: Elements) {
        element->CheckSelection(TPoint(x, y), DisplaySize);
        if (evt == Touch::TOUCH_RELEASE) {
            element->CheckClicked();
        }
    }
}

void TMainMenu::OnResized(size_t width, size_t height) {
    DisplaySize = TSize((int)width, (int)height);
}


