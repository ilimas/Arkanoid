#ifndef SHAR_H
#define SHAR_H
#include "bblock.h"
#include "blocks.h"
#include "utils.h"
#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_rect.h>
#include <SDL_ttf.h>
#include <math.h>
#include <optional>
#include <sys/types.h>

class Ball
{
    Vec2 position;
    SDL_Rect bounds;
    double gy, gx, ratio, alf;
    Vec2 destination{0.0, 1.0};
    int fx;

  public:
    int radius;

    Ball(SDL_Rect bounds_);
    bool out_of_bounds();
    bool out_of_bounds_v();
    bool out_of_bounds_h();
    void revert_position();
    std::optional<Vec2> touches(Paddel &block);
    std::optional<Vec2> touches(SingleBlock &block);
    void setgy();
    void setgx();
    void setfx(int x);
    void setalf(double x);
    void setmain();
    void Draw(SDL_Renderer *rend);
    void SDL_RenderFillCircle(SDL_Renderer *rend, int rad);
    Vec2 get_position();
    Vec2 get_destination();
    void set_destination(Vec2 new_destination);
    void set_position(Vec2 new_position);
    int retfx();
    double retalf();
    void next_step();
};
#endif
