#include "Ball.h"
#include "Paddel.h"
#include "Bricks.h"
#include "ProceduralTextures.h"
#include "Utils.h"
#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_rect.h>
#include <SDL_ttf.h>
#include <algorithm>
#include <math.h>
#include <optional>

Ball::Ball(SDL_Rect bounds_, SDL_Renderer *ren_)
{
    render = ren_;
    bounds = bounds_;
    ratio = (1.f * bounds.w) / bounds.h;
    position.x = 450;
    position.y = 585;
    destination = Vec2{0.0, -1.0 * ratio};
    radius = 10;
    alf = std::numbers::pi / 2;
    fx = 0;
    texNormal = ProceduralTextures::makeSphereTexture(render, radius * 2, SDL_Color{225, 60, 60, 255});
    texFire = ProceduralTextures::makeSphereTexture(render, radius * 2, SDL_Color{255, 165, 20, 255});
}

Ball::~Ball()
{
    SDL_DestroyTexture(texNormal);
    SDL_DestroyTexture(texFire);
}

void Ball::draw()
{
    radius = 10;
    SDL_Texture *tex = fireBallActive ? texFire : texNormal;
    SDL_Rect dst{(int)(position.x - radius), (int)(position.y - radius), radius * 2, radius * 2};
    SDL_RenderCopy(render, tex, nullptr, &dst);
}
void Ball::setgy() { destination.y = -destination.y; }
void Ball::setgx() { destination.x = -destination.x; }
int Ball::retfx() { return fx; }
double Ball::retalf() { return alf; }
void Ball::setfx(int x) { fx = x; }

void Ball::setalf(double x) { alf = x; }

bool Ball::out_of_bounds() { return out_of_bounds_h() || out_of_bounds_v(); }

bool Ball::out_of_bounds_h()
{
    double pos_x = get_position().x;
    if (std::abs(pos_x - bounds.x) <= 10 || std::abs(pos_x - bounds.w) <= 10)
    {
        return true;
    }
    return false;
}

bool Ball::out_of_bounds_v()
{
    double pos_y = get_position().y;
    if (std::abs(pos_y - bounds.y) <= 10 || std::abs(pos_y - bounds.h) <= 10)
    {
        return true;
    }
    return false;
}

void Ball::revert_position()
{
    if (out_of_bounds_h())
    {
        setgx();
        position.x = std::clamp(position.x, (double)bounds.x + radius + 1.0,
                                             (double)bounds.w - radius - 1.0);
    }
    if (out_of_bounds_v())
    {
        setgy();
        position.y = std::clamp(position.y, (double)bounds.y + radius + 1.0,
                                             (double)bounds.h - radius - 1.0);
    }
}

std::optional<Vec2> Ball::touches(Paddel &block)
{
    double pos_x = get_position().x;
    double pos_y = get_position().y;
    Vec2 paddle_vec_a{(double)block.retx(), (double)block.rety()};
    Vec2 paddle_vec_b{(double)block.retx() + block.retw(), (double)block.rety()};
    Vec2 closest_point =
        closestPointOnSegment(paddle_vec_a, paddle_vec_b, get_position());
    if ((closest_point - get_position()).length() < radius)
        return closest_point;
    return std::nullopt;
}

std::optional<Vec2> Ball::touches(SingleBlock &block)
{
    Vec2 closest_point = closestPoint(block.r, get_position());
    if ((closest_point - get_position()).length() < radius)
        return closest_point;
    return std::nullopt;
}

Vec2 Ball::get_position() { return position; }

Vec2 Ball::get_destination() { return destination; }

void Ball::set_destination(Vec2 new_destination)
{
    destination = new_destination;
}

void Ball::set_position(Vec2 new_position)
{
    position = new_position;
}

void Ball::setmain()
{
    position.x = 450;
    position.y = 585;
    destination = Vec2{0, -1.0 * ratio};
    radius = 0;
    alf = std::numbers::pi / 2;
    fx = 0;
    fireBallActive = false;
    speedElapsed = 0.0;
}

void Ball::activateFireBall(uint32_t durationMs)
{
    fireBallActive = true;
    fireBallEnd = SDL_GetTicks() + durationMs;
}

void Ball::updateFireBall()
{
    if (fireBallActive && SDL_GetTicks() >= fireBallEnd)
        fireBallActive = false;
}

void Ball::next_step(double dt)
{
    // destination's magnitude is the "canonical speed" set on paddle bounce (see
    // Game.cpp), so this is a plain px/sec speed scale, not a per-frame step -
    // the ball now covers the same distance per second regardless of frame rate.
    // Speed ramps up gradually the longer the ball stays in play (reset on
    // setmain(), i.e. each new level/life), capped so it never gets unfair.
    constexpr double baseSpeedPxPerSec = 360.0;
    constexpr double maxSpeedPxPerSec = 620.0;
    constexpr double speedRampPxPerSec2 = 4.5; // px/sec gained per second of play
    speedElapsed += dt;
    double speedPxPerSec = std::min(maxSpeedPxPerSec, baseSpeedPxPerSec + speedRampPxPerSec2 * speedElapsed);
    position += destination * (speedPxPerSec * dt);
}
