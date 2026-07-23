#ifndef FRONTEND_MANAGER_H
#define FRONTEND_MANAGER_H

#include "GLRenderer.h"
#include "PlayerDatabase.h"
#include "Starfield.h"
#include "TextRenderer.h"
#include "Types.h"
#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class FrontendManager
{
    // Caches one rasterized text texture (rendered in white; tinted at draw
    // time via GLRenderer::drawTexture's color param) so a frame that repaints
    // the same string only pays for a glyph re-rasterization when the text or
    // size actually changed.
    struct TextCache
    {
        std::string text;
        int fontSize = -1;
        GLRenderer::Texture texture{};
        Point size{0, 0};
        GLRenderer *gl{nullptr};

        ~TextCache();
        TextCache() = default;
        TextCache(const TextCache &) = delete;
        TextCache &operator=(const TextCache &) = delete;
    };

    struct DialogLine
    {
        std::string text;
        int fontSize;
        Color color;
    };

    GLRenderer::Texture tmenu;
    GLRenderer *gl;
    std::unique_ptr<TextRenderer> textRenderer_;
    std::unique_ptr<Starfield> starfield;
    uint32_t lastBackgroundTicks;

    TextCache hudScoreCache, hudLevelCache;
    TextCache menuTitleCache;
    std::array<TextCache, 4> menuButtonCache;
    TextCache settingsTitleCache;
    std::array<TextCache, 4> settingsButtonCache; // one per resolution preset, see ResolutionPresets.h
    TextCache settingsHintCache;
    std::array<TextCache, 4> dialogLineCache;
    TextCache bannerCache;
    TextCache leaderboardTitleCache, leaderboardSubtitleCache, leaderboardHintCache;
    std::array<TextCache, 10> leaderboardRowCache;

    void get_cached_texture(TextCache &cache, const std::string &utf8, int fontSize);
    void draw_cached_text(TextCache &cache, const std::string &utf8, int x, int y, Color color, int fontSize);
    void draw_cached_text_centered_x(TextCache &cache, const std::string &utf8, int centerX, int y, Color color,
                                      int fontSize);
    void draw_dim_overlay(uint8_t alpha = 140);
    void draw_dialog(const std::vector<DialogLine> &lines);
    void draw_banner(const std::string &utf8, int y, int fontSize, Color color);
    void draw_chip(TextCache &cache, const std::string &utf8, int x, int y, Color color, bool alignRight = false);

  public:
    FrontendManager(GLRenderer &gl_);
    ~FrontendManager();
    void draw_welcome_text();
    void draw_pause();
    void draw_background();
    void level_cleared(double a);
    void draw_end();
    void draw_menu(int menu);
    void draw_settings(int hoverIndex, int activeResIndex);
    void draw_name_input(const std::string &currentName);
    void draw_leaderboard(const std::vector<PlayerDatabase::Entry> &entries);
    void draw_hud(int score, int level);
};

#endif
