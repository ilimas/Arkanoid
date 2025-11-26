#ifndef PADDEL_H
#define PADDEL_H

#include <SDL.h>

class Paddel
{
    SDL_Rect nblock;
    SDL_Surface *img;
    SDL_Texture *timg;
    SDL_Renderer *render;

  public:
    Paddel(SDL_Renderer *);
    ~Paddel();
    int rety();
    void setmain();
    int retx();
    void draw();
    void setpos(int);
};

#endif
