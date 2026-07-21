#include "BlackHole.h"
#include "ProceduralTextures.h"
#include <SDL_rect.h>
#include <SDL_render.h>
#include <SDL_timer.h>
#include <cmath>
#include <cstdlib>
#include <numbers>

BlackHole::BlackHole(SDL_Rect bounds_, SDL_Renderer *render_)
{
    bounds = bounds_;
    render = render_;
    texDisk = ProceduralTextures::makeBlackHoleDiskTexture(render, DIAMETER);
    texCore = ProceduralTextures::makeBlackHoleCoreTexture(render, DIAMETER);
}

BlackHole::~BlackHole()
{
    SDL_DestroyTexture(texDisk);
    SDL_DestroyTexture(texCore);
}

void BlackHole::reset()
{
    diskAngle = 0.0f;
    lastMoveTime = SDL_GetTicks();
    auto randomInt = [](int min, int max) { return min + std::rand() % (max - min); };
    x = static_cast<float>(randomInt(30, bounds.w - DIAMETER));
    y = static_cast<float>(randomInt(30, bounds.h - DIAMETER));
    double angle = (std::rand() % 360) * std::numbers::pi / 180.0;
    vx = static_cast<float>(std::cos(angle) * SPEED);
    vy = static_cast<float>(std::sin(angle) * SPEED);
}

void BlackHole::draw()
{
    auto now = SDL_GetTicks();
    if (!animationPlaying && now - lastAnimationStartTime >= TIME_BETWEEN_ANIMATIONS)
    {
        animationPlaying = true;
        lastAnimationStartTime = now;
        reset();
    }
    if (animationPlaying)
    {
        if (now - lastAnimationStartTime >= ANIMATION_DURATION)
        {
            animationPlaying = false;
        }
        else
        {
            float dt = (now - lastMoveTime) / 1000.0f;
            lastMoveTime = now;
            x += vx * dt;
            y += vy * dt;
            if (x < 0.0f)               { x = 0.0f;                              vx = -vx; }
            if (x > bounds.w - DIAMETER) { x = static_cast<float>(bounds.w - DIAMETER); vx = -vx; }
            if (y < 0.0f)               { y = 0.0f;                              vy = -vy; }
            if (y > bounds.h - DIAMETER) { y = static_cast<float>(bounds.h - DIAMETER); vy = -vy; }

            diskAngle += dt * DISK_ROTATION_SPEED;
            if (diskAngle >= 360.0f)
                diskAngle -= 360.0f;

            SDL_Rect dst = {(int)x, (int)y, DIAMETER, DIAMETER};
            SDL_Point center{DIAMETER / 2, DIAMETER / 2};
            // Two counter-rotating passes of the same spiral texture give the
            // disk a layered, parallax-like swirl instead of a flat spin.
            SDL_RenderCopyEx(render, texDisk, nullptr, &dst, diskAngle, &center, SDL_FLIP_NONE);
            SDL_RenderCopyEx(render, texDisk, nullptr, &dst, -diskAngle * 0.6, &center, SDL_FLIP_NONE);
            SDL_RenderCopy(render, texCore, nullptr, &dst);
        }
    }
}
