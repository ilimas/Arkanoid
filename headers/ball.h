#ifndef BALL_H
#define BALL_H

#include "Bricks.h"
#include "Paddel.h"
#include "Utils.h"
#include <SDL_rect.h>
#include <SDL_render.h>
#include <optional>

class Ball
{
    Vec2 position;
    SDL_Renderer *render;
    SDL_Rect bounds;
    double gy, gx, ratio, alf;
    Vec2 destination{0.0, 1.0};
    int fx;

  public:
    int radius;

    Ball(SDL_Rect bounds_, SDL_Renderer *ren_);
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
    void draw();
    void render_circle(int rad);
    Vec2 get_position();
    Vec2 get_destination();
    void set_destination(Vec2 new_destination);
    void set_position(Vec2 new_position);
    int retfx();
    double retalf();
    void next_step();
};

#endif
