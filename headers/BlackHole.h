#ifndef BLACKHOLE_H
#define BLACKHOLE_H

#include "GLRenderer.h"
#include "Types.h"
#include <cstdint>

class BlackHole
{
  public:
    BlackHole(Rect bounds_, GLRenderer &gl_);
    ~BlackHole();
    void draw();
    void reset();
    bool isActive() const { return animationPlaying; }
    Rect getRect() const { return {(int)x, (int)y, DIAMETER, DIAMETER}; }
    void resetTimer();

  private:
    constexpr static int DIAMETER = 128;
    constexpr static uint32_t TIME_BETWEEN_ANIMATIONS = 10000; // 10 sec
    constexpr static uint32_t ANIMATION_DURATION = 5000; // 5 sec
    constexpr static float SPEED = 192.0f; // px/sec
    constexpr static float DISK_ROTATION_SPEED = 110.0f; // deg/sec
    Rect bounds;
    float x{}, y{};
    float vx{}, vy{};
    bool animationPlaying{false};
    float diskAngle{0.0f};
    uint32_t lastAnimationStartTime{0}, lastMoveTime{0};
    GLRenderer *gl = nullptr;
    GLRenderer::Texture texCore;
    GLRenderer::Texture texDisk;
};

#endif
