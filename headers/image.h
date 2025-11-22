#ifndef IMAGE_H
#define IMAGE_H
#include <SDL.h>
#include <SDL_ttf.h>
#include <stdlib.h>

class FrontendManager
{
    SDL_Surface *background, *menu, *text;
    SDL_Texture *tbackground, *tmenu, *ftext;
    SDL_Renderer *render;

  public:
    FrontendManager(SDL_Renderer *ren);
    ~FrontendManager();
    void draw_begin();
    void draw_pause();
    void draw_background();
    void level_cleared(double a);
    void draw_end();
    void draw_menu(int menu);
};
#endif
