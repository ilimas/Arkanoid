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

  private:
    constexpr static uint32_t TIME_BETWEEN_ANIMATIONS = 11000; // 11 sec
    constexpr static uint32_t ANIMATION_DURATION = 5000; // 5 sec
    SDL_Rect bounds;
    uint8_t x{}, y{};
    bool animationPlaying{false};
    uint32_t currentFrame{0};
    uint32_t lastFrameTime{0}, lastAnimationStartTime{0};
    SDL_Renderer *render = nullptr;
    std::vector<std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)>> textures{};
    std::vector<int> delays{};
    int maxFrameCount;
};

#endif
