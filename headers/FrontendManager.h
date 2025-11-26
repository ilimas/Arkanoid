#ifndef FRONTEND_MANAGER_H
#define FRONTEND_MANAGER_H

#include <SDL.h>
#include <SDL_ttf.h>
#include <stdlib.h>

class FrontendManager
{
    SDL_Surface *background, *menu, *text;
    SDL_Texture *tbackground, *tmenu, *ftext;
    SDL_Renderer *render;
    TTF_Font *fnt;

  public:
    FrontendManager(SDL_Renderer *ren);
    ~FrontendManager();
    void draw_welcome_text();
    void draw_pause();
    void draw_background();
    void level_cleared(double a);
    void draw_end();
    void draw_menu(int menu);
};

#endif
