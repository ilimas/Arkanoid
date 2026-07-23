#ifndef BALL_H
#define BALL_H

#include "Bricks.h"
#include "GLRenderer.h"
#include "Paddel.h"
#include "Types.h"
#include "Utils.h"
#include <cstdint>
#include <optional>

class Ball
{
    Vec2 position;
    GLRenderer *gl;
    Rect bounds;
    double gy, gx, ratio, alf;
    Vec2 destination{0.0, 1.0};
    int fx;
    bool fireBallActive{false};
    uint32_t fireBallEnd{0};
    GLRenderer::Texture texNormal;
    GLRenderer::Texture texFire;
    double speedElapsed{0.0}; // seconds of active play since last setmain()/sync, drives the speed ramp

  public:
    int radius;

    Ball(Rect bounds_, GLRenderer &gl_);
    ~Ball();
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
    Vec2 get_position();
    Vec2 get_destination();
    void set_destination(Vec2 new_destination);
    void set_position(Vec2 new_position);
    int retfx();
    double retalf();
    void next_step(double dt);
    double getSpeedElapsed() const { return speedElapsed; }
    void setSpeedElapsed(double t) { speedElapsed = t; }
    bool isFireBall() const { return fireBallActive; }
    void activateFireBall(uint32_t durationMs);
    void updateFireBall();
};

#endif
