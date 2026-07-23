#ifndef FRONTEND_MANAGER_H
#define FRONTEND_MANAGER_H

#include "PlayerDatabase.h"
#include "Starfield.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <array>
#include <memory>
#include <stdlib.h>
#include <string>
#include <vector>

class FrontendManager
{
    // Caches one rasterized text texture (rendered in white, tinted per-draw via
    // SDL_SetTextureColorMod) so a frame that repaints the same string only pays
    // for a glyph re-rasterization when the text or size actually changes.
    struct TextCache
    {
        std::string text;
        int fontSize = -1;
        SDL_Texture *texture = nullptr;
        SDL_Point size{0, 0};

        ~TextCache();
        TextCache() = default;
        TextCache(const TextCache &) = delete;
        TextCache &operator=(const TextCache &) = delete;
    };

    struct DialogLine
    {
        std::string text;
        int fontSize;
        SDL_Color color;
    };

    SDL_Texture *tmenu;
    SDL_Renderer *render;
    TTF_Font *fnt;
    std::unique_ptr<Starfield> starfield;
    Uint32 lastBackgroundTicks;

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

    SDL_Texture *get_cached_texture(TextCache &cache, const std::string &utf8, int fontSize, SDL_Color color);
    void draw_cached_text(TextCache &cache, const std::string &utf8, int x, int y, SDL_Color color, int fontSize);
    void draw_cached_text_centered_x(TextCache &cache, const std::string &utf8, int centerX, int y, SDL_Color color,
                                      int fontSize);
    void draw_dim_overlay(Uint8 alpha = 140);
    void draw_dialog(const std::vector<DialogLine> &lines);
    void draw_banner(const std::string &utf8, int y, int fontSize, SDL_Color color);
    void draw_chip(TextCache &cache, const std::string &utf8, int x, int y, SDL_Color color, bool alignRight = false);

  public:
    FrontendManager(SDL_Renderer *ren);
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
