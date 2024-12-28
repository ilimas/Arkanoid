#ifndef BBLOCK_H
#define BBLOCK_H
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <stdio.h>
#include <math.h>
#include <vector>
class b_block
{
    SDL_Rect nblock;
    SDL_Surface *img;
    SDL_Texture *timg;
    SDL_Renderer *render;
public:
    b_block(SDL_Renderer *);
    ~b_block();
    int rety();
    void setmain();
    int retx();
    void Draw_Block();
    void setpos(int);
};
#endif
