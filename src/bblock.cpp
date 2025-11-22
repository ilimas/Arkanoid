#include "bblock.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <stdio.h>
#include <math.h>


Paddel::Paddel(SDL_Renderer *ren)
{
    render=ren;
    img=SDL_LoadBMP(SNACKS_DIR "/img/2621.bmp");
    SDL_SetColorKey(img, 1, SDL_MapRGB(img->format, 255, 255, 255));
    timg=SDL_CreateTextureFromSurface(render, img);
    SDL_FreeSurface(img);
    nblock.x=270;
    nblock.y=460;
    nblock.w=100;
    nblock.h=10;
}
Paddel::~Paddel()
{
    SDL_DestroyTexture(timg);
}
int Paddel::rety()
{
    return nblock.y;
}
void Paddel::setmain()
{
    nblock.x=270;
    nblock.y=460;
    nblock.w=100;
    nblock.h=10;
}
int Paddel::retx()
{
    return nblock.x;
}
void Paddel::Draw_Block()
{
    SDL_RenderCopy(render, timg, NULL, &nblock);
}
void Paddel::setpos(int x)
{
    nblock.x=x;
}
