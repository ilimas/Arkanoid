#ifndef BRICKS_H
#define BRICKS_H

#include "Utils.h"
#include <SDL_render.h>
#include <cstdint>
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
    uint32_t startingSize{};
    SingleBlock temp;
    int map[26][13];
    SDL_Renderer *render;

  public:
    BlockField(SDL_Renderer *ren_);
    void draw();
    std::vector<SingleBlock> &getBlocksVector() { return a; }
    int rety(int i);
    int retx(int i);
    int retvar(int i);
    SingleBlock &retBlock(int i) { return a[i]; }
    void load_level();
    void minus(int i);
    void setmain();
    int remaining();
    void del(int i);
    uint32_t getStartingSize() { return startingSize; }
};
#endif
