#ifndef FRONTEND_MANAGER_H
#define FRONTEND_MANAGER_H

#include "PlayerDatabase.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <stdlib.h>
#include <string>

class FrontendManager
{
    SDL_Surface *background, *menu, *text;
    SDL_Texture *tbackground, *tmenu, *ftext;
    SDL_Renderer *render;
    TTF_Font *fnt;

    void draw_text(const std::string &utf8, int x, int y, SDL_Color color, int fontSize = 20);

  public:
    FrontendManager(SDL_Renderer *ren);
    ~FrontendManager();
    void draw_welcome_text();
    void draw_pause();
    void draw_background();
    void level_cleared(double a);
    void draw_end();
    void draw_menu(int menu);
    void draw_name_input(const std::string &currentName);
    void draw_leaderboard(const std::vector<PlayerDatabase::Entry> &entries);
};

#endif
