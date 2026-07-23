#ifndef UI_LAYOUT_H
#define UI_LAYOUT_H

#include <SDL_rect.h>

// Shared layout constants for menu buttons so the hit-testing in EventManager
// always matches what FrontendManager actually draws.
namespace UILayout
{
constexpr int ScreenW = 1440;
constexpr int ScreenH = 992;

// Tall enough for 4 rows of buttons (main menu: Новая игра/Выход/Таблица/
// Настройки; settings screen reuses the same panel + button rects for its 4
// resolution presets).
constexpr SDL_Rect MenuPanel{768, 224, 608, 652};
constexpr int MenuTitleY = MenuPanel.y + 54;

constexpr int MenuButtonW = 480;
constexpr int MenuButtonH = 83;
constexpr int MenuButtonSpacing = 29;
constexpr int MenuButtonX = MenuPanel.x + (MenuPanel.w - MenuButtonW) / 2;
constexpr int MenuButtonStartY = MenuPanel.y + 192;

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
