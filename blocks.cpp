#include <SDL.h>
#include <SDL_TTF.h>
#include <SDL_mixer.h>
#include <stdio.h>
#include <math.h>
#include <windows.h>
#include <vector>
#include <stdlib.h>
#include <time.h>
#include <fstream>
#include "blocks.h"
using namespace std;
blocks::blocks()
{
    for(int i=0 ; i<26 ; i++)
    {
        for(int j=0 ; j<13 ; j++)
        {
            map[i][j]=0;
        }
    }
}
int blocks::rety(int i)
{
    return a[i].r.y;
}
int blocks::retx(int i)
{
    return a[i].r.x;
}
int blocks::bsize()
{
    return (int)a.size();
}
void blocks::del(int i)
{
    a.erase(a.begin()+i);
}
void blocks::setmain()
{
    a.clear();
}
void blocks::load_level()
{
    int i,j, k;
    temp.r.h=15;
    temp.r.w=50;
    for(i=0 ; i<20 ; i++)
    {
        for(j=0 ; j<13 ; j++)
        {
            k=rand()%8;
            if(k<=2) map[i][j]=k;
            else map[i][j]=0;
        }
    }
    for(i=0 ; i<26 ; i++)
    {
        for(j=0 ; j<13 ; j++)
        {
            if(map[i][j]==1)
            {
                temp.r.y=i*15;
                temp.r.x=j*50;
                temp.var=1;
                a.push_back(temp);
            }
            else if(map[i][j]==2)
            {
                temp.r.y=i*15;
                temp.r.x=j*50;
                temp.var=2;
                a.push_back(temp);
            }
        }
    }
}
int blocks::retvar(int i)
{
    return a[i].var;
}
void blocks::minus(int i)
{
    a[i].var--;
}
void blocks::Draw_Blocks(SDL_Renderer *ren)
{
    SDL_Rect r;
    for(int i=0 ; i<(int)a.size(); i++)
    {
        if(a[i].var==1) SDL_SetRenderDrawColor(ren, 202, 255, 112, 255);
        else if(a[i].var==2) SDL_SetRenderDrawColor(ren, 255, 202, 112, 255);
        SDL_RenderFillRect(ren, &a[i].r);
    }
    for(int i=0 ; i<(int)a.size(); i++)
    {
        if(a[i].var==1) SDL_SetRenderDrawColor(ren, 0, 255, 0, 255);
        else if(a[i].var==2) SDL_SetRenderDrawColor(ren, 255, 0, 0, 255);
        r.x=a[i].r.x+2;
        r.y=a[i].r.y+2;
        r.h=13;
        r.w=48;
        SDL_RenderFillRect(ren, &r);
    }
}
