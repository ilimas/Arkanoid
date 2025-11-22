#ifndef BLOCKS_H
#define BLOCKS_H
#include "utils.h"
#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <cstdint>
#include <math.h>
#include <vector>

enum class HitSide : uint8_t
{
    HIT_NONE,
    HIT_LEFT,
    HIT_RIGHT,
    HIT_TOP,
    HIT_BOTTOM
};

class Ball;

class SingleBlock
{
  public:
    SDL_Rect r;
    int var;

    HitSide detectHitSide(const Vec2 &ballPos, double ball_radius);
    Vec2 reflectBall(Ball &ball);
};

class BlockField
{
    std::vector<SingleBlock> a;
    SingleBlock temp;
    int map[26][13];

  public:
    BlockField();
    void Draw_Blocks(SDL_Renderer *ren);
    std::vector<SingleBlock> &getBlocksVector()
    {
        return a;
    }
    int rety(int i);
    int retx(int i);
    int retvar(int i);
    SingleBlock &retBlock(int i)
    {
        return a[i];
    }
    void load_level();
    void minus(int i);
    void setmain();
    int bsize();
    void del(int i);
};
#endif
