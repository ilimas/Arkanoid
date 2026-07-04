#include "Bricks.h"
#include "Ball.h"
#include "Utils.h"
#include <SDL.h>
#include <SDL_rect.h>
#include <cstdlib>

BlockField::BlockField(SDL_Renderer *ren_)
{
    render = ren_;
    for (int i = 0; i < 26; i++)
        for (int j = 0; j < 16; j++)
            map[i][j] = 0;
}

int BlockField::rety(int i) { return a[i].r.y; }
int BlockField::retx(int i) { return a[i].r.x; }
int BlockField::remaining() { return (int)a.size(); }

void BlockField::del(int i) { a.erase(a.begin() + i); }
void BlockField::setmain() { a.clear(); }

int BlockField::destructibleCount()
{
    int count = 0;
    for (auto &b : a)
        if (b.var > 0) count++;
    return count;
}

void BlockField::load_level(int levelNum)
{
    a.clear();
    for (int i = 0; i < 26; i++)
        for (int j = 0; j < 16; j++)
            map[i][j] = 0;

    switch (levelNum % 6)
    {
    case 0: // Stripes
        for (int j = 0; j < 16; j++)
        {
            map[0][j] = 1; map[2][j] = 1;
            map[4][j] = 2; map[6][j] = 2;
        }
        break;

    case 1: // Pyramid (wide at top, narrow at bottom)
        for (int i = 0; i < 8; i++)
            for (int j = i; j < 16 - i; j++)
                map[i][j] = (i < 4) ? 1 : 2;
        break;

    case 2: // Diamond
        for (int i = 0; i < 8; i++)
        {
            int half = (i < 4) ? i : 7 - i;
            int lo = 7 - half, hi = 8 + half;
            for (int j = lo; j <= hi; j++)
                map[i][j] = (j == lo || j == hi || i == 0 || i == 7) ? 2 : 1;
        }
        break;

    case 3: // Vertical stone pillars with normal bricks between
        for (int i = 0; i < 8; i++)
            for (int j = 0; j < 16; j++)
            {
                if (j % 4 == 0)
                    map[i][j] = -1; // stone at cols 0, 4, 8, 12
                else
                    map[i][j] = ((i + j / 4) % 2 == 0) ? 1 : 2;
            }
        break;

    case 4: // Checkerboard
        for (int i = 0; i < 7; i++)
            for (int j = 0; j < 16; j++)
                map[i][j] = ((i + j) % 2 == 0) ? 1 : 2;
        break;

    case 5: // Hard random with stones
        for (int i = 0; i < 12; i++)
            for (int j = 0; j < 16; j++)
            {
                int r = std::rand() % 10;
                if (r == 0)      map[i][j] = -1; // 10% stone
                else if (r <= 3) map[i][j] = 2;  // 30% double-hit
                else if (r <= 6) map[i][j] = 1;  // 30% single-hit
                // else 30% empty
            }
        break;
    }

    temp.r.h = 15;
    temp.r.w = 50;
    for (int i = 0; i < 26; i++)
        for (int j = 0; j < 16; j++)
        {
            if (map[i][j] == 0) continue;
            temp.r.y = i * 15 + 30; // 30px top margin keeps blocks away from wall-bounce zone
            temp.r.x = j * 50 + 50; // 50px left margin centers 16 cols in 900px screen
            temp.var = map[i][j];   // -1=stone, 1=single-hit, 2=double-hit
            a.push_back(temp);
        }

    startingSize = (uint32_t)a.size();
    destructibleStartingSize = (uint32_t)destructibleCount();
}

int BlockField::retvar(int i) { return a[i].var; }

void BlockField::minus(int i) { a[i].var--; }

void BlockField::draw()
{
    SDL_Rect r;
    for (int i = 0; i < (int)a.size(); i++)
    {
        if (a[i].var > 0 && a[i].var == 1)
            SDL_SetRenderDrawColor(render, 202, 255, 112, 255);
        else if (a[i].var > 0 && a[i].var == 2)
            SDL_SetRenderDrawColor(render, 255, 202, 112, 255);
        else if (a[i].var < 0) // stone
            SDL_SetRenderDrawColor(render, 80, 80, 90, 255);
        SDL_RenderFillRect(render, &a[i].r);
    }
    for (int i = 0; i < (int)a.size(); i++)
    {
        if (a[i].var > 0 && a[i].var == 1)
            SDL_SetRenderDrawColor(render, 0, 255, 0, 255);
        else if (a[i].var > 0 && a[i].var == 2)
            SDL_SetRenderDrawColor(render, 255, 0, 0, 255);
        else if (a[i].var < 0) // stone
            SDL_SetRenderDrawColor(render, 140, 140, 155, 255);
        r.x = a[i].r.x + 2;
        r.y = a[i].r.y + 2;
        r.h = 13;
        r.w = 48;
        SDL_RenderFillRect(render, &r);
    }
}

HitSide SingleBlock::detectHitSide(const Vec2 &ballPos, double radius)
{
    Vec2 closest_point = closestPoint(this->r, ballPos);
    float dx = ballPos.x - closest_point.x;
    float dy = ballPos.y - closest_point.y;
    if (std::fabs(dx) > std::fabs(dy))
        return (dx > 0) ? HitSide::HIT_LEFT : HitSide::HIT_RIGHT;
    else
        return (dy > 0) ? HitSide::HIT_TOP : HitSide::HIT_BOTTOM;
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
        v.x = -v.x; break;
    case HitSide::HIT_TOP:
    case HitSide::HIT_BOTTOM:
        v.y = -v.y; break;
    default:
        break;
    }
    return v;
}
