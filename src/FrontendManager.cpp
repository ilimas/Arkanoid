#include "FrontendManager.h"
#include "Clock.h"
#include "ResolutionPresets.h"
#include "UILayout.h"
#include "stb_image.h"
#include <algorithm>
#include <cstdio>
#include <iostream>

namespace
{
constexpr Color kGold{255, 185, 15, 255};
constexpr Color kPink{255, 52, 179, 255};
constexpr Color kText{230, 232, 245, 255};
constexpr Color kPanelBg{12, 16, 34, 235};
constexpr Color kChipBg{10, 14, 30, 175};
} // namespace

FrontendManager::TextCache::~TextCache()
{
    if (gl && texture.valid())
        gl->destroyTexture(texture);
}

FrontendManager::FrontendManager(GLRenderer &gl_)
{
    gl = &gl_;
    textRenderer_ = std::make_unique<TextRenderer>(SNACKS_DIR "/ttf/Casper_B.ttf");

    starfield = std::make_unique<Starfield>(*gl, UILayout::ScreenW, UILayout::ScreenH);
    lastBackgroundTicks = nowMs();

    int mw, mh, mchannels;
    unsigned char *menuPixels = stbi_load(SNACKS_DIR "/img/menu.bmp", &mw, &mh, &mchannels, 4);
    if (menuPixels)
    {
        tmenu = gl->createTextureFromPixels(menuPixels, mw, mh);
        stbi_image_free(menuPixels);
    }
    else
    {
        std::cerr << "Failed to load menu.bmp: " << stbi_failure_reason() << "\n";
    }
}

FrontendManager::~FrontendManager() { gl->destroyTexture(tmenu); }

// Rasterizes `utf8` at `fontSize` only when the text or size actually changed since
// the cache was last filled; the glyph texture is always rendered in white so a
// pure color change (e.g. menu hover) is just a different tint at draw time, not a
// re-rasterization.
void FrontendManager::get_cached_texture(TextCache &cache, const std::string &utf8, int fontSize)
{
    if (!cache.texture.valid() || cache.text != utf8 || cache.fontSize != fontSize)
    {
        if (cache.texture.valid())
            gl->destroyTexture(cache.texture);
        TextRenderer::Bitmap bmp = textRenderer_->render(utf8, fontSize);
        if (bmp.w > 0 && bmp.h > 0)
        {
            cache.texture = gl->createTextureFromPixels(bmp.rgba.data(), bmp.w, bmp.h);
            cache.size = Point{cache.texture.w, cache.texture.h};
        }
        else
        {
            cache.size = Point{0, 0};
        }
        cache.text = utf8;
        cache.fontSize = fontSize;
        cache.gl = gl;
    }
}

void FrontendManager::draw_cached_text(TextCache &cache, const std::string &utf8, int x, int y, Color color,
                                        int fontSize)
{
    get_cached_texture(cache, utf8, fontSize);
    if (!cache.texture.valid())
        return;
    Rect r{x, y, cache.size.x, cache.size.y};
    gl->drawTexture(cache.texture, r, color);
}

void FrontendManager::draw_cached_text_centered_x(TextCache &cache, const std::string &utf8, int centerX, int y,
                                                    Color color, int fontSize)
{
    get_cached_texture(cache, utf8, fontSize);
    draw_cached_text(cache, utf8, centerX - cache.size.x / 2, y, color, fontSize);
}

void FrontendManager::draw_dim_overlay(uint8_t alpha)
{
    gl->drawColor(Rect{0, 0, UILayout::ScreenW, UILayout::ScreenH}, Color{0, 0, 0, alpha});
}

void FrontendManager::draw_dialog(const std::vector<DialogLine> &lines)
{
    constexpr int lineSpacing = 22;
    constexpr int paddingX = 80;
    constexpr int paddingY = 58;

    int maxW = 0;
    int totalH = 0;
    for (size_t i = 0; i < lines.size() && i < dialogLineCache.size(); ++i)
    {
        get_cached_texture(dialogLineCache[i], lines[i].text, lines[i].fontSize);
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

    Rect panel{panelX, panelY, panelW, panelH};
    gl->drawColor(panel, kPanelBg);
    gl->drawColorOutline(panel, kGold);
    Rect innerPanel{panelX + 3, panelY + 3, panelW - 6, panelH - 6};
    gl->drawColorOutline(innerPanel, Color{kGold.r, kGold.g, kGold.b, 90});

    int centerX = UILayout::ScreenW / 2;
    int y = panelY + paddingY;
    for (size_t i = 0; i < lines.size() && i < dialogLineCache.size(); ++i)
    {
        draw_cached_text(dialogLineCache[i], lines[i].text, centerX - dialogLineCache[i].size.x / 2, y,
                          lines[i].color, lines[i].fontSize);
        y += dialogLineCache[i].size.y + lineSpacing;
    }
}

void FrontendManager::draw_banner(const std::string &utf8, int y, int fontSize, Color color)
{
    constexpr int paddingX = 38;
    constexpr int paddingY = 19;
    get_cached_texture(bannerCache, utf8, fontSize);
    int barW = bannerCache.size.x + paddingX * 2;
    int barH = bannerCache.size.y + paddingY * 2;
    int barX = (UILayout::ScreenW - barW) / 2;
    int barY = y - paddingY;

    Rect bar{barX, barY, barW, barH};
    gl->drawColor(bar, kChipBg);
    gl->drawColorOutline(bar, Color{kGold.r, kGold.g, kGold.b, 200});

    draw_cached_text_centered_x(bannerCache, utf8, UILayout::ScreenW / 2, y, color, fontSize);
}

void FrontendManager::draw_chip(TextCache &cache, const std::string &utf8, int x, int y, Color color,
                                 bool alignRight)
{
    constexpr int fontSize = 32;
    constexpr int paddingX = 22;
    constexpr int paddingY = 13;
    get_cached_texture(cache, utf8, fontSize);
    int chipW = cache.size.x + paddingX * 2;
    int chipH = cache.size.y + paddingY * 2;
    int chipX = alignRight ? x - chipW : x;

    Rect r{chipX, y, chipW, chipH};
    gl->drawColor(r, kChipBg);
    gl->drawColorOutline(r, Color{color.r, color.g, color.b, 220});

    draw_cached_text(cache, utf8, chipX + paddingX, y + paddingY, color, fontSize);
}

void FrontendManager::draw_welcome_text()
{
    draw_banner("Нажмите левую кнопку мыши, чтобы начать", 768, 35, kGold);
}

void FrontendManager::draw_pause()
{
    draw_dialog({
        {"Игра на паузе", 54, kGold},
        {"Esc - продолжить", 32, kText},
        {"Enter - выход в меню", 32, kText},
    });
}

void FrontendManager::draw_background()
{
    uint32_t now = nowMs();
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
        {"Уровень пройден!", 54, kGold},
        {buf, 32, kText},
        {"Enter - в меню", 29, kPink},
    });
}

void FrontendManager::draw_end()
{
    draw_dialog({
        {"Игра окончена", 54, kGold},
        {"Enter - в меню", 32, kPink},
    });
}

void FrontendManager::draw_menu(int menu)
{
    gl->drawTexture(tmenu, Rect{0, 0, UILayout::ScreenW, UILayout::ScreenH});

    const Rect &panel = UILayout::MenuPanel;
    gl->drawColor(panel, kPanelBg);
    gl->drawColorOutline(panel, kGold);

    draw_cached_text_centered_x(menuTitleCache, "АРКАНОИД", panel.x + panel.w / 2, UILayout::MenuTitleY, kGold, 58);

    static const char *labels[4] = {"Новая игра", "Выход", "Таблица игроков", "Настройки"};
    for (int i = 0; i < 4; i++)
    {
        Rect btn = UILayout::MenuButtonRect(i);
        bool selected = (i == menu);
        if (selected)
            gl->drawColor(btn, Color{kGold.r, kGold.g, kGold.b, 235});
        else
            gl->drawColor(btn, Color{255, 255, 255, 30});
        gl->drawColorOutline(btn, Color{kGold.r, kGold.g, kGold.b, 200});

        Color labelColor = selected ? Color{20, 16, 10, 255} : kPink;
        get_cached_texture(menuButtonCache[i], labels[i], 38);
        int lx = btn.x + (btn.w - menuButtonCache[i].size.x) / 2;
        int ly = btn.y + (btn.h - menuButtonCache[i].size.y) / 2;
        draw_cached_text(menuButtonCache[i], labels[i], lx, ly, labelColor, 38);
    }
}

void FrontendManager::draw_settings(int hoverIndex, int activeResIndex)
{
    gl->drawTexture(tmenu, Rect{0, 0, UILayout::ScreenW, UILayout::ScreenH});

    const Rect &panel = UILayout::MenuPanel;
    gl->drawColor(panel, kPanelBg);
    gl->drawColorOutline(panel, kGold);

    draw_cached_text_centered_x(settingsTitleCache, "НАСТРОЙКИ", panel.x + panel.w / 2, UILayout::MenuTitleY, kGold,
                                 58);

    constexpr Color kActiveGreen{140, 255, 190, 255};
    for (int i = 0; i < kResolutionPresetCount; i++)
    {
        Rect btn = UILayout::MenuButtonRect(i);
        bool hovered = (i == hoverIndex);
        bool active = (i == activeResIndex);
        if (hovered)
            gl->drawColor(btn, Color{kGold.r, kGold.g, kGold.b, 235});
        else if (active)
            gl->drawColor(btn, Color{kActiveGreen.r, kActiveGreen.g, kActiveGreen.b, 70});
        else
            gl->drawColor(btn, Color{255, 255, 255, 30});
        Color borderColor = (active && !hovered) ? Color{kActiveGreen.r, kActiveGreen.g, kActiveGreen.b, 200}
                                                  : Color{kGold.r, kGold.g, kGold.b, 200};
        gl->drawColorOutline(btn, borderColor);

        std::string label = kResolutionPresets[i].label;
        if (active)
            label += " (текущее)"; // marks the currently applied resolution
        Color labelColor = hovered ? Color{20, 16, 10, 255} : (active ? kActiveGreen : kPink);
        get_cached_texture(settingsButtonCache[i], label, 32);
        int lx = btn.x + (btn.w - settingsButtonCache[i].size.x) / 2;
        int ly = btn.y + (btn.h - settingsButtonCache[i].size.y) / 2;
        draw_cached_text(settingsButtonCache[i], label, lx, ly, labelColor, 32);
    }

    draw_cached_text_centered_x(settingsHintCache, "Esc / Enter - назад в меню", panel.x + panel.w / 2,
                                 panel.y + panel.h - 40, kPink, 26);
}

void FrontendManager::draw_name_input(const std::string &currentName)
{
    std::string display = currentName + "_";
    draw_dialog({
        {"Введите имя игрока:", 38, kGold},
        {display, 48, kText},
        {"Enter - начать игру, Esc - в меню", 29, kPink},
    });
}

void FrontendManager::draw_leaderboard(const std::vector<PlayerDatabase::Entry> &entries)
{
    constexpr int panelW = 896;
    int rowCount = std::max<int>(1, std::min<int>(10, (int)entries.size()));
    constexpr int rowH = 48;
    int panelH = 160 + rowCount * rowH + 80;
    int panelX = (UILayout::ScreenW - panelW) / 2;
    int panelY = (UILayout::ScreenH - panelH) / 2;

    draw_dim_overlay();
    Rect panel{panelX, panelY, panelW, panelH};
    gl->drawColor(panel, kPanelBg);
    gl->drawColorOutline(panel, kGold);

    int centerX = UILayout::ScreenW / 2;
    draw_cached_text_centered_x(leaderboardTitleCache, "Таблица игроков", centerX, panelY + 38, kGold, 42);
    draw_cached_text_centered_x(leaderboardSubtitleCache, "(всего сбито блоков)", centerX, panelY + 90, kPink, 26);

    int y = panelY + 160;
    int rank = 1;
    for (const auto &e : entries)
    {
        if (rank > 10)
            break;
        std::string line = std::to_string(rank) + ". " + e.name + " - " + std::to_string(e.totalBlocks);
        draw_cached_text_centered_x(leaderboardRowCache[rank - 1], line, centerX, y, kText, 32);
        y += rowH;
        rank++;
    }
    if (entries.empty())
        draw_cached_text_centered_x(leaderboardRowCache[0], "Пока нет результатов", centerX, y, kText, 32);

    draw_cached_text_centered_x(leaderboardHintCache, "Enter / Esc / клик - назад в меню", centerX,
                                 panelY + panelH - 54, kPink, 26);
}

void FrontendManager::draw_hud(int score, int level)
{
    char buf[64];
    snprintf(buf, sizeof(buf), "Счёт: %d", score);
    draw_chip(hudScoreCache, buf, 26, 19, kGold);
    snprintf(buf, sizeof(buf), "Уровень: %d", level + 1);
    draw_chip(hudLevelCache, buf, UILayout::ScreenW - 26, 19, kGold, true);
}
