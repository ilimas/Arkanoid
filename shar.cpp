#include "shar.h"
#include <SDL.h>
#include <SDL_TTF.h>
#include <SDL_mixer.h>
#include <stdio.h>
#include <math.h>
#include <windows.h>
#include <vector>
shar::shar()
{
    RECT.x=320;
    RECT.y=448;
    gy=1;
    gx=2;
    radius=0;
    alf=4;
    fx=0;
}
void shar::Draw(SDL_Renderer *rend)
{
    radius=10;
    SDL_SetRenderDrawColor(rend, 192, 192, 192, 255);
    SDL_RenderFillCircle(rend, radius);
    SDL_SetRenderDrawColor(rend, 255, 0, 0, 255);
    radius=8;
    SDL_RenderFillCircle(rend, radius);
}
void shar::setgy()
{
    gy=-gy;
}
void shar::setgx()
{
    gx=-gx;
}
int shar::retfx()
{
    return fx;
}
int shar::retalf()
{
    return alf;
}
void shar::setfx(int x)
{
    fx=x;
}

void shar::setalf(int x)
{
    alf=x;
}
void shar::SDL_RenderFillCircle(SDL_Renderer* rend, int rad)
{
    int x = rad;
    int y = 0;
    int radiusError = 1 - x;
    while (x >= y)
    {
        SDL_RenderDrawLine(rend, x + RECT.x, y + RECT.y, -x + RECT.x, y + RECT.y);
        SDL_RenderDrawLine(rend, y + RECT.x, x + RECT.y, -y + RECT.x, x + RECT.y);
        SDL_RenderDrawLine(rend, -x + RECT.x, -y + RECT.y, x + RECT.x, -y + RECT.y);
        SDL_RenderDrawLine(rend, -y + RECT.x, -x + RECT.y, y + RECT.x, -x + RECT.y);
        y++;
        if (radiusError < 0)
            radiusError += 2 * y + 1;
        else
        {
            x--;
            radiusError += 2 * (y - x + 1);
        }
    }
}
int shar::rety()
{
    return RECT.y;
}
int shar::retx()
{
    return RECT.x;
}
int shar::retgx()
{
    return gx;
}
int shar::retgy()
{
    return gy;
}
void shar::setmain()
{
    RECT.x=320;
    RECT.y=448;
    gy=1;
    gx=2;
    radius=0;
    alf=4;
    fx=0;
}
void shar::next_step()
{
    RECT.x+=2*gx*fx;
    if(!fx) RECT.y-=alf*gy*(fx+1);
    else RECT.y-=alf*gy;
}
