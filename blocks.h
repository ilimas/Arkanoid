#ifndef BLOCKS_H
#define BLOCKS_H
#include <SDL.h>
#include <SDL_TTF.h>
#include <SDL_mixer.h>
#include <stdio.h>
#include <math.h>
#include <windows.h>
#include <vector>
using namespace std;
class blocks
{
    struct qwe
    {
        SDL_Rect r;
        int var;
    };
    vector <qwe> a;
    qwe temp;
    int map[26][13];

public:
    blocks();
    void Draw_Blocks(SDL_Renderer *ren);
    int rety(int i);
    int retx(int i);
    int retvar(int i);
    void load_level();
    void minus(int i);
    void setmain();
    int bsize();
    void del(int i);
};
#endif
