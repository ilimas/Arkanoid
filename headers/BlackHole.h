#ifndef BLACKHOLE_H
#define BLACKHOLE_H

#include <SDL_rect.h>
#include <SDL_render.h>
#include <SDL_timer.h>
#include <cstdint>

class BlackHole
{
  public:
    BlackHole(SDL_Rect bounds_, SDL_Renderer *render);
    ~BlackHole();
    void draw();
    void reset();
    bool isActive() const { return animationPlaying; }
    SDL_Rect getRect() const { return {(int)x, (int)y, DIAMETER, DIAMETER}; }
    void resetTimer() { animationPlaying = false; lastAnimationStartTime = SDL_GetTicks(); }

  private:
    constexpr static int DIAMETER = 128;
    constexpr static uint32_t TIME_BETWEEN_ANIMATIONS = 10000; // 10 sec
    constexpr static uint32_t ANIMATION_DURATION = 5000; // 5 sec
    constexpr static float SPEED = 192.0f; // px/sec
    constexpr static float DISK_ROTATION_SPEED = 110.0f; // deg/sec
    SDL_Rect bounds;
    float x{}, y{};
    float vx{}, vy{};
    bool animationPlaying{false};
    float diskAngle{0.0f};
    uint32_t lastAnimationStartTime{0}, lastMoveTime{0};
    SDL_Renderer *render = nullptr;
    SDL_Texture *texCore{nullptr};
    SDL_Texture *texDisk{nullptr};
    float totalTime_{0.0f};

    // Real-time GLSL path (core + photon ring + swirling accretion disk, all
    // computed per-pixel every frame, rotation driven by totalTime_): used
    // when GLInterop::available() and this program compiled successfully;
    // falls back to the CPU-baked texCore/texDisk path above otherwise.
    unsigned int shaderProgram_{0};
    SDL_Texture *shaderTarget_{nullptr};
};

#endif
