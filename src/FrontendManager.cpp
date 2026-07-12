#include "FrontendManager.h"
#include "UILayout.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_render.h>
#include <SDL_rwops.h>
#include <SDL_stdinc.h>
#include <SDL_ttf.h>
#include <algorithm>
#include <cstdio>
#include <iostream>
#include <stdexcept>

namespace
{
constexpr SDL_Color kGold{255, 185, 15, 255};
constexpr SDL_Color kPink{255, 52, 179, 255};
constexpr SDL_Color kText{230, 232, 245, 255};
constexpr SDL_Color kPanelBg{12, 16, 34, 235};
constexpr SDL_Color kChipBg{10, 14, 30, 175};
} // namespace

FrontendManager::TextCache::~TextCache()
{
    if (texture)
        SDL_DestroyTexture(texture);
}

FrontendManager::FrontendManager(SDL_Renderer *ren)
{
    if (TTF_Init() != 0)
    {
        std::cerr << "TTF_Init error\n";
    }
    fnt = TTF_OpenFont(SNACKS_DIR "/ttf/Casper_B.ttf", 30);
    if (!fnt)
        throw std::runtime_error("Unable to load font");
    render = ren;
    SDL_SetRenderDrawBlendMode(render, SDL_BLENDMODE_BLEND);

    starfield = std::make_unique<Starfield>(render, UILayout::ScreenW, UILayout::ScreenH);
    lastBackgroundTicks = SDL_GetTicks();

    SDL_Surface *menuSurf = SDL_LoadBMP(SNACKS_DIR "/img/menu.bmp");
    tmenu = SDL_CreateTextureFromSurface(ren, menuSurf);
    SDL_FreeSurface(menuSurf);
}

FrontendManager::~FrontendManager()
{
    TTF_CloseFont(fnt);
    fnt = nullptr;
    TTF_Quit();
    SDL_DestroyTexture(tmenu);
}

// Rasterizes `utf8` at `fontSize` only when the text or size actually changed since
// the cache was last filled; the glyph texture is always rendered in white so a
// pure color change (e.g. menu hover) is a cheap SDL_SetTextureColorMod, not a
// TTF re-rasterization. TTF_SetFontSize is comparatively expensive (rebuilds
// FreeType hinting state), so this matters for anything redrawn every frame.
SDL_Texture *FrontendManager::get_cached_texture(TextCache &cache, const std::string &utf8, int fontSize,
                                                  SDL_Color color)
{
    if (!cache.texture || cache.text != utf8 || cache.fontSize != fontSize)
    {
        if (cache.texture)
        {
            SDL_DestroyTexture(cache.texture);
            cache.texture = nullptr;
        }
        TTF_SetFontSize(fnt, fontSize);
        SDL_Surface *surf = TTF_RenderUTF8_Blended(fnt, utf8.c_str(), SDL_Color{255, 255, 255, 255});
        TTF_SetFontSize(fnt, 30);
        if (surf)
        {
            cache.texture = SDL_CreateTextureFromSurface(render, surf);
            cache.size = SDL_Point{surf->w, surf->h};
            SDL_FreeSurface(surf);
        }
        else
        {
            cache.size = SDL_Point{0, 0};
        }
        cache.text = utf8;
        cache.fontSize = fontSize;
    }
    if (cache.texture)
    {
        SDL_SetTextureColorMod(cache.texture, color.r, color.g, color.b);
        SDL_SetTextureAlphaMod(cache.texture, color.a);
    }
    return cache.texture;
}

void FrontendManager::draw_cached_text(TextCache &cache, const std::string &utf8, int x, int y, SDL_Color color,
                                        int fontSize)
{
    SDL_Texture *tex = get_cached_texture(cache, utf8, fontSize, color);
    if (!tex)
        return;
    SDL_Rect r{x, y, cache.size.x, cache.size.y};
    SDL_RenderCopy(render, tex, NULL, &r);
}

void FrontendManager::draw_cached_text_centered_x(TextCache &cache, const std::string &utf8, int centerX, int y,
                                                    SDL_Color color, int fontSize)
{
    get_cached_texture(cache, utf8, fontSize, color);
    draw_cached_text(cache, utf8, centerX - cache.size.x / 2, y, color, fontSize);
}

void FrontendManager::draw_dim_overlay(Uint8 alpha)
{
    SDL_SetRenderDrawColor(render, 0, 0, 0, alpha);
    SDL_Rect full{0, 0, UILayout::ScreenW, UILayout::ScreenH};
    SDL_RenderFillRect(render, &full);
}

void FrontendManager::draw_dialog(const std::vector<DialogLine> &lines)
{
    constexpr int lineSpacing = 14;
    constexpr int paddingX = 50;
    constexpr int paddingY = 36;

    int maxW = 0;
    int totalH = 0;
    for (size_t i = 0; i < lines.size() && i < dialogLineCache.size(); ++i)
    {
        get_cached_texture(dialogLineCache[i], lines[i].text, lines[i].fontSize, lines[i].color);
        maxW = std::max(maxW, dialogLineCache[i].size.x);
        totalH += dialogLineCache[i].size.y + lineSpacing;
    }
    if (!lines.empty())
        totalH -= lineSpacing;

    int panelW = maxW + paddingX * 2;
    int panelH = totalH + paddingY * 2;
    int panelX = (UILayout::ScreenW - panelW) / 2;
    int panelY = (UILayout::ScreenH - panelH) / 2;

    draw_dim_overlay();

    SDL_SetRenderDrawColor(render, kPanelBg.r, kPanelBg.g, kPanelBg.b, kPanelBg.a);
    SDL_Rect panel{panelX, panelY, panelW, panelH};
    SDL_RenderFillRect(render, &panel);
    SDL_SetRenderDrawColor(render, kGold.r, kGold.g, kGold.b, kGold.a);
    SDL_RenderDrawRect(render, &panel);
    SDL_Rect innerPanel{panelX + 3, panelY + 3, panelW - 6, panelH - 6};
    SDL_SetRenderDrawColor(render, kGold.r, kGold.g, kGold.b, 90);
    SDL_RenderDrawRect(render, &innerPanel);

    int centerX = UILayout::ScreenW / 2;
    int y = panelY + paddingY;
    for (size_t i = 0; i < lines.size() && i < dialogLineCache.size(); ++i)
    {
        draw_cached_text(dialogLineCache[i], lines[i].text, centerX - dialogLineCache[i].size.x / 2, y,
                          lines[i].color, lines[i].fontSize);
        y += dialogLineCache[i].size.y + lineSpacing;
    }
}

void FrontendManager::draw_banner(const std::string &utf8, int y, int fontSize, SDL_Color color)
{
    constexpr int paddingX = 24;
    constexpr int paddingY = 12;
    get_cached_texture(bannerCache, utf8, fontSize, color);
    int barW = bannerCache.size.x + paddingX * 2;
    int barH = bannerCache.size.y + paddingY * 2;
    int barX = (UILayout::ScreenW - barW) / 2;
    int barY = y - paddingY;

    SDL_SetRenderDrawColor(render, kChipBg.r, kChipBg.g, kChipBg.b, kChipBg.a);
    SDL_Rect bar{barX, barY, barW, barH};
    SDL_RenderFillRect(render, &bar);
    SDL_SetRenderDrawColor(render, kGold.r, kGold.g, kGold.b, 200);
    SDL_RenderDrawRect(render, &bar);

    draw_cached_text_centered_x(bannerCache, utf8, UILayout::ScreenW / 2, y, color, fontSize);
}

void FrontendManager::draw_chip(TextCache &cache, const std::string &utf8, int x, int y, SDL_Color color,
                                 bool alignRight)
{
    constexpr int fontSize = 20;
    constexpr int paddingX = 14;
    constexpr int paddingY = 8;
    get_cached_texture(cache, utf8, fontSize, color);
    int chipW = cache.size.x + paddingX * 2;
    int chipH = cache.size.y + paddingY * 2;
    int chipX = alignRight ? x - chipW : x;

    SDL_SetRenderDrawColor(render, kChipBg.r, kChipBg.g, kChipBg.b, kChipBg.a);
    SDL_Rect r{chipX, y, chipW, chipH};
    SDL_RenderFillRect(render, &r);
    SDL_SetRenderDrawColor(render, color.r, color.g, color.b, 220);
    SDL_RenderDrawRect(render, &r);

    draw_cached_text(cache, utf8, chipX + paddingX, y + paddingY, color, fontSize);
}

void FrontendManager::draw_welcome_text()
{
    draw_banner("Нажмите левую кнопку мыши, чтобы начать", 480, 22, kGold);
}

void FrontendManager::draw_pause()
{
    draw_dialog({
        {"Игра на паузе", 34, kGold},
        {"Esc - продолжить", 20, kText},
        {"Enter - выход в меню", 20, kText},
    });
}

void FrontendManager::draw_background()
{
    Uint32 now = SDL_GetTicks();
    double dt = std::min((now - lastBackgroundTicks) / 1000.0, 0.05);
    lastBackgroundTicks = now;
    starfield->update(dt);
    starfield->draw();
}

void FrontendManager::level_cleared(double a)
{
    char buf[64];
    snprintf(buf, sizeof(buf), "Ваш ранк: %.2f", a);
    draw_dialog({
        {"Уровень пройден!", 34, kGold},
        {buf, 20, kText},
        {"Enter - в меню", 18, kPink},
    });
}

void FrontendManager::draw_end()
{
    draw_dialog({
        {"Игра окончена", 34, kGold},
        {"Enter - в меню", 20, kPink},
    });
}

void FrontendManager::draw_menu(int menu)
{
    SDL_RenderCopy(render, tmenu, NULL, NULL);

    const SDL_Rect &panel = UILayout::MenuPanel;
    SDL_SetRenderDrawColor(render, kPanelBg.r, kPanelBg.g, kPanelBg.b, kPanelBg.a);
    SDL_RenderFillRect(render, &panel);
    SDL_SetRenderDrawColor(render, kGold.r, kGold.g, kGold.b, kGold.a);
    SDL_RenderDrawRect(render, &panel);

    draw_cached_text_centered_x(menuTitleCache, "АРКАНОИД", panel.x + panel.w / 2, UILayout::MenuTitleY, kGold, 36);

    static const char *labels[3] = {"Новая игра", "Выход", "Таблица игроков"};
    for (int i = 0; i < 3; i++)
    {
        SDL_Rect btn = UILayout::MenuButtonRect(i);
        bool selected = (i == menu);
        if (selected)
        {
            SDL_SetRenderDrawColor(render, kGold.r, kGold.g, kGold.b, 235);
            SDL_RenderFillRect(render, &btn);
        }
        else
        {
            SDL_SetRenderDrawColor(render, 255, 255, 255, 30);
            SDL_RenderFillRect(render, &btn);
        }
        SDL_SetRenderDrawColor(render, kGold.r, kGold.g, kGold.b, 200);
        SDL_RenderDrawRect(render, &btn);

        SDL_Color labelColor = selected ? SDL_Color{20, 16, 10, 255} : kPink;
        get_cached_texture(menuButtonCache[i], labels[i], 24, labelColor);
        int lx = btn.x + (btn.w - menuButtonCache[i].size.x) / 2;
        int ly = btn.y + (btn.h - menuButtonCache[i].size.y) / 2;
        draw_cached_text(menuButtonCache[i], labels[i], lx, ly, labelColor, 24);
    }
}

void FrontendManager::draw_name_input(const std::string &currentName)
{
    std::string display = currentName + "_";
    draw_dialog({
        {"Введите имя игрока:", 24, kGold},
        {display, 30, kText},
        {"Enter - начать игру, Esc - в меню", 18, kPink},
    });
}

void FrontendManager::draw_leaderboard(const std::vector<PlayerDatabase::Entry> &entries)
{
    constexpr int panelW = 560;
    int rowCount = std::max<int>(1, std::min<int>(10, (int)entries.size()));
    constexpr int rowH = 30;
    int panelH = 100 + rowCount * rowH + 50;
    int panelX = (UILayout::ScreenW - panelW) / 2;
    int panelY = (UILayout::ScreenH - panelH) / 2;

    draw_dim_overlay();
    SDL_SetRenderDrawColor(render, kPanelBg.r, kPanelBg.g, kPanelBg.b, kPanelBg.a);
    SDL_Rect panel{panelX, panelY, panelW, panelH};
    SDL_RenderFillRect(render, &panel);
    SDL_SetRenderDrawColor(render, kGold.r, kGold.g, kGold.b, kGold.a);
    SDL_RenderDrawRect(render, &panel);

    int centerX = UILayout::ScreenW / 2;
    draw_cached_text_centered_x(leaderboardTitleCache, "Таблица игроков", centerX, panelY + 24, kGold, 26);
    draw_cached_text_centered_x(leaderboardSubtitleCache, "(всего сбито блоков)", centerX, panelY + 56, kPink, 16);

    int y = panelY + 100;
    int rank = 1;
    for (const auto &e : entries)
    {
        if (rank > 10)
            break;
        std::string line = std::to_string(rank) + ". " + e.name + " - " + std::to_string(e.totalBlocks);
        draw_cached_text_centered_x(leaderboardRowCache[rank - 1], line, centerX, y, kText, 20);
        y += rowH;
        rank++;
    }
    if (entries.empty())
        draw_cached_text_centered_x(leaderboardRowCache[0], "Пока нет результатов", centerX, y, kText, 20);

    draw_cached_text_centered_x(leaderboardHintCache, "Enter / Esc / клик - назад в меню", centerX,
                                 panelY + panelH - 34, kPink, 16);
}

void FrontendManager::draw_hud(int score, int level)
{
    char buf[64];
    snprintf(buf, sizeof(buf), "Счёт: %d", score);
    draw_chip(hudScoreCache, buf, 16, 12, kGold);
    snprintf(buf, sizeof(buf), "Уровень: %d", level + 1);
    draw_chip(hudLevelCache, buf, UILayout::ScreenW - 16, 12, kGold, true);
}
