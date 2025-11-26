#include "Ball.h"
#include "Paddel.h"
#include "Bricks.h"
#include "Utils.h"
#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_rect.h>
#include <SDL_ttf.h>
#include <math.h>
#include <optional>

Ball::Ball(SDL_Rect bounds_, SDL_Renderer *ren_)
{
    render = ren_;
    bounds = bounds_;
    ratio = (1.f * bounds.w) / bounds.h;
    position.x = 320;
    position.y = 448;
    destination = Vec2{0.0, -1.0 * ratio};
    radius = 10;
    alf = std::numbers::pi / 2;
    fx = 0;
}
void Ball::draw()
{
    radius = 10;
    SDL_SetRenderDrawColor(render, 192, 192, 192, 255);
    render_circle(radius);
    radius = 8;
    SDL_SetRenderDrawColor(render, 255, 0, 0, 255);
    render_circle(radius);
    radius = 10;
}
void Ball::setgy() { destination.y = -destination.y; }
void Ball::setgx() { destination.x = -destination.x; }
int Ball::retfx() { return fx; }
double Ball::retalf() { return alf; }
void Ball::setfx(int x) { fx = x; }

void Ball::setalf(double x) { alf = x; }
void Ball::render_circle(int rad)
{
    int x = rad;
    int y = 0;
    int radiusError = 1 - x;
    while (x >= y)
    {
        SDL_RenderDrawLine(render, x + position.x, y + position.y, -x + position.x,
                           y + position.y);
        SDL_RenderDrawLine(render, y + position.x, x + position.y, -y + position.x,
                           x + position.y);
        SDL_RenderDrawLine(render, -x + position.x, -y + position.y, x + position.x,
                           -y + position.y);
        SDL_RenderDrawLine(render, -y + position.x, -x + position.y, y + position.x,
                           -x + position.y);
        y++;
        if (radiusError < 0)
            radiusError += 2 * y + 1;
        else
        {
            x--;
            radiusError += 2 * (y - x + 1);
        }
    }
}

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
        setgx();
    if (out_of_bounds_v())
        setgy();
}

std::optional<Vec2> Ball::touches(Paddel &block)
{
    double pos_x = get_position().x;
    double pos_y = get_position().y;
    Vec2 paddle_vec_a{(double)block.retx(), (double)block.rety()};
    Vec2 paddle_vec_b{(double)block.retx() + 100, (double)block.rety()};
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
    position.x = 320;
    position.y = 448;
    destination = Vec2{0, -1.0 * ratio};
    radius = 0;
    alf = std::numbers::pi / 2;
    fx = 0;
}

void Ball::next_step()
{
    constexpr float boost = 6.0;
    position += boost * destination;
}
