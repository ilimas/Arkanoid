#include "Paddel.h"
#include "ProceduralTextures.h"
#include <SDL.h>
#include <SDL_ttf.h>


Paddel::Paddel(SDL_Renderer *ren)
{
    render=ren;
    nblock.x=640;
    nblock.y=960;
    nblock.w=192;
    nblock.h=16;
    timg = ProceduralTextures::makeBarTexture(render, nblock.w, nblock.h, SDL_Color{70, 190, 255, 255});
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
    nblock.x=640;
    nblock.y=960;
    nblock.w=192;
    nblock.h=16;
}
int Paddel::retx()
{
    return nblock.x;
}
void Paddel::draw()
{
    SDL_RenderCopy(render, timg, NULL, &nblock);
}
void Paddel::setpos(int x)
{
    nblock.x=x;
}
