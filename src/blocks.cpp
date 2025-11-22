#include "blocks.h"
#include "shar.h"
#include "utils.h"
#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_rect.h>
#include <SDL_ttf.h>
#include <math.h>
#include <stdlib.h>
#include <vector>

BlockField::BlockField()
{
    for (int i = 0; i < 26; i++)
    {
        for (int j = 0; j < 13; j++)
        {
            map[i][j] = 0;
        }
    }
}
int BlockField::rety(int i)
{
    return a[i].r.y;
}
int BlockField::retx(int i)
{
    return a[i].r.x;
}
int BlockField::bsize()
{
    return (int)a.size();
}
void BlockField::del(int i)
{
    a.erase(a.begin() + i);
}
void BlockField::setmain()
{
    a.clear();
}
void BlockField::load_level()
{
    int i, j, k;
    temp.r.h = 15;
    temp.r.w = 50;
    for (i = 0; i < 20; i++)
    {
        for (j = 0; j < 13; j++)
        {
            k = rand() % 8;
            if (k <= 2)
                map[i][j] = k;
            else
                map[i][j] = 0;
        }
    }
    for (i = 0; i < 26; i++)
    {
        for (j = 0; j < 13; j++)
        {
            if (map[i][j] == 1)
            {
                temp.r.y = i * 15;
                temp.r.x = j * 50;
                temp.var = 1;
                a.push_back(temp);
            }
            else if (map[i][j] == 2)
            {
                temp.r.y = i * 15;
                temp.r.x = j * 50;
                temp.var = 2;
                a.push_back(temp);
            }
        }
    }
}
int BlockField::retvar(int i)
{
    return a[i].var;
}
void BlockField::minus(int i)
{
    a[i].var--;
}
void BlockField::Draw_Blocks(SDL_Renderer *ren)
{
    SDL_Rect r;
    for (int i = 0; i < (int)a.size(); i++)
    {
        if (a[i].var == 1)
            SDL_SetRenderDrawColor(ren, 202, 255, 112, 255);
        else if (a[i].var == 2)
            SDL_SetRenderDrawColor(ren, 255, 202, 112, 255);
        SDL_RenderFillRect(ren, &a[i].r);
    }
    for (int i = 0; i < (int)a.size(); i++)
    {
        if (a[i].var == 1)
            SDL_SetRenderDrawColor(ren, 0, 255, 0, 255);
        else if (a[i].var == 2)
            SDL_SetRenderDrawColor(ren, 255, 0, 0, 255);
        r.x = a[i].r.x + 2;
        r.y = a[i].r.y + 2;
        r.h = 13;
        r.w = 48;
        SDL_RenderFillRect(ren, &r);
    }
}

HitSide SingleBlock::detectHitSide(const Vec2 &ballPos, double radius)
{

    Vec2 closest_point = closestPoint(this->r, ballPos);

    float dx = ballPos.x - closest_point.x;
    float dy = ballPos.y - closest_point.y;

    if (std::fabs(dx) > std::fabs(dy))
    {
        if (dx > 0)
            return HitSide::HIT_LEFT;
        else
            return HitSide::HIT_RIGHT;
    }
    else
    {
        if (dy > 0)
            return HitSide::HIT_TOP;
        else
            return HitSide::HIT_BOTTOM;
    }

    return HitSide::HIT_NONE;
}

Vec2 SingleBlock::reflectBall(Ball &ball)
{
    Vec2 v = ball.get_destination();
    auto side = detectHitSide(ball.get_position(), ball.radius);

    switch (side)
    {
    case HitSide::HIT_LEFT:
    case HitSide::HIT_RIGHT:
        v.x = -v.x;
        break;

    case HitSide::HIT_TOP:
    case HitSide::HIT_BOTTOM:
        v.y = -v.y;
        break;

    default:
        break;
    }

    return v;
}
