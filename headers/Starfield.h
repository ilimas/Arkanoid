#ifndef STARFIELD_H
#define STARFIELD_H

#include "GLRenderer.h"
#include "Types.h"
#include <vector>

// Procedurally animated space background: a shaded nebula wash, twinkling
// stars, and comets that periodically streak across - drawn instead of a
// static background image.
class Starfield
{
  public:
    Starfield(GLRenderer &gl_, int width, int height);
    ~Starfield();

    void update(double dt);
    void draw();

  private:
    struct Star
    {
        float x, y;
        float size;
        float baseAlpha;
        float twinkleSpeed;
        float twinklePhase;
    };

    struct Comet
    {
        float x, y;
        float vx, vy;
        float life;
        Color color;
    };

    void spawnComet();

    GLRenderer *gl;
    int width_, height_;
    GLRenderer::Texture nebulaTex;
    GLRenderer::Texture cometTex;
    std::vector<Star> stars;
    std::vector<Comet> comets;
    float timeSinceLastComet{0.0f};
    float nextCometDelay{2.0f};
};

#endif
