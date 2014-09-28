#pragma once

#include <functional>

#include "display.h"
#include "world.h"

class TMenuElement {
public:
    explicit TMenuElement(const TPointF& position,
                          const std::string& text,
                          std::function<void()> onClick)
        : Position(position)
        , Text(text)
        , OnClick(onClick)
        , Selected(false)
        , Font(gameplay::Font::create("res/font-menu.gpb"))
    {
    }
    ~TMenuElement() {
        SAFE_RELEASE(Font);
    }
    void Draw(const TSize& displaySize);
    void CheckSelection(const TPoint& mousePosition, const TSize& displaySize);
    void CheckClicked();
private:
    TPointF Position;
    std::string Text;
    std::function<void()> OnClick;
    bool Selected;
    gameplay::Font* Font;
};

typedef std::shared_ptr<TMenuElement> TMenuElementRef;

class TMainMenu : public IDrawable, public IControlable {
public:
    explicit TMainMenu(std::function<void()> onExit,
                       std::function<void()> onQuickGame);
    void Draw(float) override;
    void OnTouchEvent(gameplay::Touch::TouchEvent evt, int x, int y, unsigned int contactIndex) override;
    void OnResized(size_t width, size_t height) override;
private:
    std::function<void()> OnExit;
    std::function<void()> OnQuickGame;
    std::vector<TMenuElementRef> Elements;
    TSize DisplaySize;
};
