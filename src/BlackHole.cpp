#include "BlackHole.h"
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
    auto *black_hole_animation = IMG_LoadAnimation(SNACKS_DIR "/img/blackhole.gif");
    for (size_t i = 0; i < black_hole_animation->count; ++i)
    {
        decltype(textures)::value_type currentFrame = {
            SDL_CreateTextureFromSurface(render, black_hole_animation->frames[i]),
            &SDL_DestroyTexture};
        textures.emplace_back(std::move(currentFrame));
        delays.push_back(black_hole_animation->delays[i]);
    }
    maxFrameCount = black_hole_animation->count;
    IMG_FreeAnimation(black_hole_animation);
}

void BlackHole::reset()
{
    currentFrame = 0;
    lastFrameTime = lastMoveTime = SDL_GetTicks();
    auto randomInt = [](int min, int max) { return min + std::rand() % (max - min); };
    x = static_cast<float>(randomInt(30, bounds.w - 80));
    y = static_cast<float>(randomInt(30, bounds.h - 80));
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
            if (x < 0.0f)          { x = 0.0f;                         vx = -vx; }
            if (x > bounds.w - 80) { x = static_cast<float>(bounds.w - 80); vx = -vx; }
            if (y < 0.0f)          { y = 0.0f;                         vy = -vy; }
            if (y > bounds.h - 80) { y = static_cast<float>(bounds.h - 80); vy = -vy; }

            if (now - lastFrameTime >= static_cast<uint32_t>(delays[currentFrame]))
            {
                currentFrame = (currentFrame + 1) % maxFrameCount;
                lastFrameTime = now;
            }
            SDL_Rect dst = {(int)x, (int)y, 80, 80};
            SDL_RenderCopy(render, textures[currentFrame].get(), nullptr, &dst);
        }
    }
}
