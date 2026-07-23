#include "Ball.h"
#include "GLInterop.h"
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

namespace
{
const char *kBallVertexShader = R"GLSL(
#version 120
attribute vec2 aPos;
attribute vec2 aUV;
varying vec2 vUV;
void main()
{
    vUV = aUV;
    gl_Position = vec4(aPos, 0.0, 1.0);
}
)GLSL";

// Glossy shaded sphere with an offset specular highlight that drifts subtly
// over time - replaces the CPU-baked makeSphereTexture look with a per-pixel,
// per-frame version so the highlight never looks perfectly static.
const char *kBallFragmentShader = R"GLSL(
#version 120
varying vec2 vUV;
uniform vec2 uResolution;
uniform float uTime;
uniform vec3 uBaseColor;

void main()
{
    float r = uResolution.x * 0.5;
    vec2 center = uResolution * 0.5;
    vec2 px = vUV * uResolution;
    float dist = length(px - center);
    if (dist > r + 0.5)
    {
        gl_FragColor = vec4(0.0);
        return;
    }

    float pulse = 0.05 * sin(uTime * 2.2);
    vec2 hl = center - vec2(r * (0.35 + pulse), r * 0.35);
    float distToHl = length(px - hl);
    float highlight = max(0.0, 1.0 - distToHl / (r * 0.85));
    float rim = max(0.0, (dist / r - 0.7) / 0.3);
    float f = 0.6 + 0.9 * highlight - 0.35 * rim;

    vec3 col = uBaseColor * f;
    float alpha = clamp(1.0 - max(0.0, dist - (r - 1.0)), 0.0, 1.0);
    gl_FragColor = vec4(col, alpha);
}
)GLSL";
} // namespace

Ball::Ball(SDL_Rect bounds_, SDL_Renderer *ren_)
{
    render = ren_;
    bounds = bounds_;
    ratio = (1.f * bounds.w) / bounds.h;
    position.x = 720;
    position.y = 936;
    destination = Vec2{0.0, -1.0 * ratio};
    radius = 16;
    alf = std::numbers::pi / 2;
    fx = 0;
    texNormal = ProceduralTextures::makeSphereTexture(render, radius * 2, SDL_Color{225, 60, 60, 255});
    texFire = ProceduralTextures::makeSphereTexture(render, radius * 2, SDL_Color{255, 165, 20, 255});

    if (GLInterop::available())
    {
        shaderProgram_ = GLInterop::compileProgram(kBallVertexShader, kBallFragmentShader);
        if (shaderProgram_)
        {
            shaderTarget_ = SDL_CreateTexture(render, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
                                               radius * 2, radius * 2);
            if (shaderTarget_)
                SDL_SetTextureBlendMode(shaderTarget_, SDL_BLENDMODE_BLEND);
            else
            {
                GLInterop::destroyProgram(shaderProgram_);
                shaderProgram_ = 0;
            }
        }
    }
}

Ball::~Ball()
{
    SDL_DestroyTexture(texNormal);
    SDL_DestroyTexture(texFire);
    if (shaderTarget_)
        SDL_DestroyTexture(shaderTarget_);
    GLInterop::destroyProgram(shaderProgram_);
}

void Ball::draw()
{
    radius = 16;
    SDL_Rect dst{(int)(position.x - radius), (int)(position.y - radius), radius * 2, radius * 2};

    if (GLInterop::available() && shaderProgram_ && shaderTarget_)
    {
        SDL_Color base = fireBallActive ? SDL_Color{255, 165, 20, 255} : SDL_Color{225, 60, 60, 255};
        float t = SDL_GetTicks() / 1000.0f;

        SDL_Texture *prevTarget = SDL_GetRenderTarget(render);
        SDL_SetRenderTarget(render, shaderTarget_);
        GLInterop::setViewport(radius * 2, radius * 2);
        GLInterop::setUniform2f(GLInterop::getUniformLocation(shaderProgram_, "uResolution"), (float)(radius * 2),
                                 (float)(radius * 2));
        GLInterop::setUniform1f(GLInterop::getUniformLocation(shaderProgram_, "uTime"), t);
        GLInterop::setUniform3f(GLInterop::getUniformLocation(shaderProgram_, "uBaseColor"), base.r / 255.0f,
                                 base.g / 255.0f, base.b / 255.0f);
        GLInterop::drawFullscreenQuad(shaderProgram_, "aPos", "aUV");
        SDL_SetRenderTarget(render, prevTarget);

        SDL_RenderCopy(render, shaderTarget_, nullptr, &dst);
        return;
    }

    SDL_Texture *tex = fireBallActive ? texFire : texNormal;
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
    if (std::abs(pos_x - bounds.x) <= 16 || std::abs(pos_x - bounds.w) <= 16)
    {
        return true;
    }
    return false;
}

bool Ball::out_of_bounds_v()
{
    double pos_y = get_position().y;
    if (std::abs(pos_y - bounds.y) <= 16 || std::abs(pos_y - bounds.h) <= 16)
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
    position.x = 720;
    position.y = 936;
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
    constexpr double baseSpeedPxPerSec = 576.0;
    constexpr double maxSpeedPxPerSec = 992.0;
    constexpr double speedRampPxPerSec2 = 7.2; // px/sec gained per second of play
    speedElapsed += dt;
    double speedPxPerSec = std::min(maxSpeedPxPerSec, baseSpeedPxPerSec + speedRampPxPerSec2 * speedElapsed);
    position += destination * (speedPxPerSec * dt);
}
