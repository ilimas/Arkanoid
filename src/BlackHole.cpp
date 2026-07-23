#include "BlackHole.h"
#include "Clock.h"
#include "ProceduralTextures.h"
#include <cmath>
#include <cstdlib>
#include <numbers>

BlackHole::BlackHole(Rect bounds_, GLRenderer &gl_)
{
    bounds = bounds_;
    gl = &gl_;
    texDisk = ProceduralTextures::makeBlackHoleDiskTexture(*gl, DIAMETER);
    texCore = ProceduralTextures::makeBlackHoleCoreTexture(*gl, DIAMETER);
}

BlackHole::~BlackHole()
{
    gl->destroyTexture(texDisk);
    gl->destroyTexture(texCore);
}

void BlackHole::resetTimer()
{
    animationPlaying = false;
    lastAnimationStartTime = nowMs();
}

void BlackHole::reset()
{
    diskAngle = 0.0f;
    lastMoveTime = nowMs();
    auto randomInt = [](int min, int max) { return min + std::rand() % (max - min); };
    x = static_cast<float>(randomInt(48, bounds.w - DIAMETER));
    y = static_cast<float>(randomInt(48, bounds.h - DIAMETER));
    double angle = (std::rand() % 360) * std::numbers::pi / 180.0;
    vx = static_cast<float>(std::cos(angle) * SPEED);
    vy = static_cast<float>(std::sin(angle) * SPEED);
}

void BlackHole::draw()
{
    auto now = nowMs();
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

            Rect dst = {(int)x, (int)y, DIAMETER, DIAMETER};
            // Two counter-rotating passes of the same spiral texture give the
            // disk a layered, parallax-like swirl instead of a flat spin.
            gl->drawTexture(texDisk, dst, Color{255, 255, 255, 255}, diskAngle);
            gl->drawTexture(texDisk, dst, Color{255, 255, 255, 255}, -diskAngle * 0.6);
            gl->drawTexture(texCore, dst);
        }
    }
}
