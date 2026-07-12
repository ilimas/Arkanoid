#ifndef UI_LAYOUT_H
#define UI_LAYOUT_H

#include <SDL_rect.h>

// Shared layout constants for menu buttons so the hit-testing in EventManager
// always matches what FrontendManager actually draws.
namespace UILayout
{
constexpr int ScreenW = 900;
constexpr int ScreenH = 620;

constexpr SDL_Rect MenuPanel{480, 140, 380, 340};
constexpr int MenuTitleY = MenuPanel.y + 34;

constexpr int MenuButtonW = 300;
constexpr int MenuButtonH = 52;
constexpr int MenuButtonSpacing = 18;
constexpr int MenuButtonX = MenuPanel.x + (MenuPanel.w - MenuButtonW) / 2;
constexpr int MenuButtonStartY = MenuPanel.y + 120;

constexpr SDL_Rect MenuButtonRect(int index)
{
    return SDL_Rect{MenuButtonX, MenuButtonStartY + index * (MenuButtonH + MenuButtonSpacing), MenuButtonW,
                     MenuButtonH};
}

constexpr bool PointInRect(int px, int py, const SDL_Rect &r)
{
    return px >= r.x && px < r.x + r.w && py >= r.y && py < r.y + r.h;
}
} // namespace UILayout

#endif
