#ifndef PADDEL_H
#define PADDEL_H

#include "GLRenderer.h"
#include "Types.h"

class Paddel
{
    Rect nblock;
    GLRenderer::Texture timg;
    GLRenderer *gl;

  public:
    Paddel(GLRenderer &gl_);
    ~Paddel();
    int rety();
    void setmain();
    int retx();
    int retw() { return nblock.w; }
    void draw();
    void setpos(int);
};

#endif
