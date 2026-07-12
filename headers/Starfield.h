#ifndef STARFIELD_H
#define STARFIELD_H

#include <SDL.h>
#include <vector>

// Procedurally animated space background: a shaded nebula wash, twinkling
// stars, and comets that periodically streak across - drawn instead of a
// static background image.
class Starfield
{
  public:
    Starfield(SDL_Renderer *ren, int width, int height);
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
        SDL_Color color;
    };

    void spawnComet();

    SDL_Renderer *render;
    int width_, height_;
    SDL_Texture *nebulaTex;
    SDL_Texture *cometTex;
    std::vector<Star> stars;
    std::vector<Comet> comets;
    float timeSinceLastComet{0.0f};
    float nextCometDelay{2.0f};
};

#endif
