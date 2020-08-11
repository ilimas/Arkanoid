#include "bblock.h"
#include <SDL.h>
#include <SDL_TTF.h>
#include <SDL_mixer.h>
#include <stdio.h>
#include <math.h>
#include <windows.h>
#include <vector>
b_block::b_block(SDL_Renderer *ren)
{
    render=ren;
    img=SDL_LoadBMP("2621.bmp");
    SDL_SetColorKey(img, 1, SDL_MapRGB(img->format, 255, 255, 255));
    timg=SDL_CreateTextureFromSurface(render, img);
    SDL_FreeSurface(img);
    nblock.x=270;
    nblock.y=460;
    nblock.w=100;
    nblock.h=10;
}
b_block::~b_block()
{
    SDL_DestroyTexture(timg);
}
int b_block::rety()
{
    return nblock.y;
}
void b_block::setmain()
{
    nblock.x=270;
    nblock.y=460;
    nblock.w=100;
    nblock.h=10;
}
int b_block::retx()
{
    return nblock.x;
}
void b_block::Draw_Block()
{
    SDL_RenderCopy(render, timg, NULL, &nblock);
}
void b_block::setpos(int x)
{
    nblock.x=x;
}
