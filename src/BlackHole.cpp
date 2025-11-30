#include "BlackHole.h"
#include <SDL_rect.h>
#include <SDL_render.h>
#include <SDL_timer.h>

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
    lastFrameTime = SDL_GetTicks();
    auto randomInt = [](int min, int max) { return min + std::rand() % (max - min); };
    x = randomInt(30, bounds.w - 80);
    y = randomInt(30, bounds.h - 80);
}

void BlackHole::draw()
{
    static constexpr double speed_factor = 1.2;
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
            if (now - lastFrameTime >= delays[currentFrame])
            {
                currentFrame = (currentFrame + 1) % maxFrameCount;
                lastFrameTime = now;
            }
            SDL_Rect dst = {x, y, 80, 80};
            SDL_RenderCopy(render, textures[currentFrame].get(), nullptr, &dst);
        }
    }
}
