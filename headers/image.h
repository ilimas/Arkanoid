#ifndef IMAGE_H
#define IMAGE_H
#include <SDL.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>

class image
{
    SDL_Surface *background, *menu, *text;
    SDL_Texture *tbackground, *tmenu, *ftext;
    SDL_Renderer *render;
public:
    image(SDL_Renderer *ren);
    ~image();
    void draw_begin();
    void draw_pause();
    void draw_background();
    void level_cleared(double a);
    void draw_end();
    void draw_menu(int menu);
};
#endif
