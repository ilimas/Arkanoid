#include "Paddel.h"
#include "ProceduralTextures.h"


Paddel::Paddel(GLRenderer &gl_)
{
    gl=&gl_;
    nblock.x=640;
    nblock.y=960;
    nblock.w=192;
    nblock.h=16;
    timg = ProceduralTextures::makeBarTexture(*gl, nblock.w, nblock.h, Color{70, 190, 255, 255});
}
Paddel::~Paddel()
{
    gl->destroyTexture(timg);
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
    gl->drawTexture(timg, nblock);
}
void Paddel::setpos(int x)
{
    nblock.x=x;
}
