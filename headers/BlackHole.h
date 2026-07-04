#ifndef BLACKHOLE_H
#define BLACKHOLE_H

#include <SDL_image.h>
#include <SDL_rect.h>
#include <SDL_render.h>
#include <SDL_timer.h>
#include <cstdint>
#include <memory>
#include <vector>

class BlackHole
{
  public:
    BlackHole(SDL_Rect bounds_, SDL_Renderer *render);
    ~BlackHole() = default;
    void draw();
    void reset();
    bool isActive() const { return animationPlaying; }
    SDL_Rect getRect() const { return {(int)x, (int)y, 80, 80}; }
    void resetTimer() { animationPlaying = false; lastAnimationStartTime = SDL_GetTicks(); }

  private:
    constexpr static uint32_t TIME_BETWEEN_ANIMATIONS = 10000; // 10 sec
    constexpr static uint32_t ANIMATION_DURATION = 5000; // 5 sec
    constexpr static float SPEED = 120.0f; // px/sec
    SDL_Rect bounds;
    float x{}, y{};
    float vx{}, vy{};
    bool animationPlaying{false};
    uint32_t currentFrame{0};
    uint32_t lastFrameTime{0}, lastAnimationStartTime{0}, lastMoveTime{0};
    SDL_Renderer *render = nullptr;
    std::vector<std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)>> textures{};
    std::vector<int> delays{};
    int maxFrameCount;
};

#endif
