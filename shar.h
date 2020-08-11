#ifndef SHAR_H
#define SHAR_H
#include <SDL.h>
#include <SDL_TTF.h>
#include <SDL_mixer.h>
#include <stdio.h>
#include <math.h>
#include <windows.h>
#include <vector>
class shar
{
    SDL_Rect RECT;
    int gy, gx, alf, fx, radius;
public:
    shar();
    void setgy();
    void setgx();
    void setfx(int x);
    void setalf(int x);
    void setmain();
    void Draw(SDL_Renderer *rend);
    void SDL_RenderFillCircle(SDL_Renderer* rend, int rad);
    int rety();
    int retx();
    int retgx();
    int retalf();
    int retgy();
    int retfx();
    void next_step();
};
#endif
