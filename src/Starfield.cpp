#include "Starfield.h"
#include "ProceduralTextures.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace
{
float frand(float lo, float hi) { return lo + (hi - lo) * (std::rand() / (float)RAND_MAX); }
} // namespace

Starfield::Starfield(GLRenderer &gl_, int width, int height) : gl(&gl_), width_(width), height_(height)
{
    nebulaTex = ProceduralTextures::makeNebulaTexture(*gl, width_, height_);
    cometTex = ProceduralTextures::makeCometTexture(*gl, 144, 8, Color{200, 220, 255, 255});

    constexpr int kStarCount = 160;
    stars.reserve(kStarCount);
    for (int i = 0; i < kStarCount; i++)
    {
        Star s;
        s.x = frand(0.0f, (float)width_);
        s.y = frand(0.0f, (float)height_);
        float depth = frand(0.0f, 1.0f);
        s.size = 1.6f + depth * 2.6f;
        s.baseAlpha = 90.0f + depth * 165.0f;
        s.twinkleSpeed = frand(0.6f, 2.2f);
        s.twinklePhase = frand(0.0f, 6.2831853f);
        stars.push_back(s);
    }
    nextCometDelay = frand(1.5f, 4.0f);
}

Starfield::~Starfield()
{
    gl->destroyTexture(nebulaTex);
    gl->destroyTexture(cometTex);
}

void Starfield::spawnComet()
{
    static const Color palette[3] = {
        {200, 220, 255, 255},
        {255, 235, 210, 255},
        {215, 255, 235, 255},
    };

    Comet c;
    bool fromLeft = std::rand() % 2 == 0;
    float speed = frand(416.0f, 672.0f);
    // Angle in radians measured the way the rotated draw rotates (clockwise in
    // screen space): ~15-32 deg below horizontal, heading down-right from the
    // left edge or down-left from the right edge.
    float steepness = frand(0.25f, 0.55f);
    float angle = fromLeft ? steepness : (float)M_PI - steepness;
    c.x = fromLeft ? frand(-96.0f, width_ * 0.3f) : frand(width_ * 0.7f, width_ + 96.0f);
    c.y = frand(-96.0f, height_ * 0.15f);
    c.vx = std::cos(angle) * speed;
    c.vy = std::sin(angle) * speed;
    c.color = palette[std::rand() % 3];
    c.life = 6.0f;
    comets.push_back(c);
}

void Starfield::update(double dt)
{
    for (auto &s : stars)
        s.twinklePhase += (float)dt * s.twinkleSpeed;

    timeSinceLastComet += (float)dt;
    if (timeSinceLastComet >= nextCometDelay)
    {
        spawnComet();
        timeSinceLastComet = 0.0f;
        nextCometDelay = frand(2.5f, 6.0f);
    }

    for (size_t i = 0; i < comets.size();)
    {
        Comet &c = comets[i];
        c.x += c.vx * (float)dt;
        c.y += c.vy * (float)dt;
        c.life -= (float)dt;
        bool offscreen = c.x < -160.0f || c.x > width_ + 160.0f || c.y < -160.0f || c.y > height_ + 160.0f;
        if (c.life <= 0.0f || offscreen)
            comets.erase(comets.begin() + (long)i);
        else
            ++i;
    }
}

void Starfield::draw()
{
    gl->drawTexture(nebulaTex, Rect{0, 0, width_, height_});

    for (auto &s : stars)
    {
        uint8_t twinkle = (uint8_t)std::clamp(s.baseAlpha * (0.55f + 0.45f * std::sin(s.twinklePhase)), 0.0f, 255.0f);
        Rect r{(int)s.x, (int)s.y, (int)std::ceil(s.size), (int)std::ceil(s.size)};
        gl->drawColor(r, Color{235, 240, 255, twinkle});
    }

    for (auto &c : comets)
    {
        float angleDeg = std::atan2(c.vy, c.vx) * 180.0f / (float)M_PI;
        Rect dst{(int)c.x - cometTex.w, (int)c.y - cometTex.h / 2, cometTex.w, cometTex.h};
        gl->drawTexture(cometTex, dst, c.color, angleDeg, 1.0f, 0.5f);
    }
}
