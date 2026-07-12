#include "ProceduralTextures.h"
#include <algorithm>
#include <cmath>

namespace
{
Uint8 clamp8(float v) { return (Uint8)std::clamp(v, 0.0f, 255.0f); }

SDL_Color shade(SDL_Color base, float factor)
{
    return SDL_Color{clamp8(base.r * factor), clamp8(base.g * factor), clamp8(base.b * factor), base.a};
}

SDL_Surface *newSurface(int w, int h) { return SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_RGBA32); }

SDL_Texture *finish(SDL_Renderer *ren, SDL_Surface *surf)
{
    SDL_Texture *tex = SDL_CreateTextureFromSurface(ren, surf);
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    SDL_FreeSurface(surf);
    return tex;
}
} // namespace

namespace ProceduralTextures
{

SDL_Texture *makeBrickTexture(SDL_Renderer *ren, int w, int h, SDL_Color base)
{
    SDL_Surface *surf = newSurface(w, h);
    Uint32 *px = static_cast<Uint32 *>(surf->pixels);
    const SDL_PixelFormat *fmt = surf->format;
    constexpr int bevel = 2;
    constexpr float corner = 2.5f;

    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            bool cut = false;
            auto cutCorner = [&](float cxp, float cyp) {
                float dx = x - cxp, dy = y - cyp;
                return dx * dx + dy * dy > corner * corner;
            };
            if (x < corner && y < corner)
                cut = cutCorner(corner, corner);
            else if (x >= w - corner && y < corner)
                cut = cutCorner(w - 1 - corner, corner);
            else if (x < corner && y >= h - corner)
                cut = cutCorner(corner, h - 1 - corner);
            else if (x >= w - corner && y >= h - corner)
                cut = cutCorner(w - 1 - corner, h - 1 - corner);

            float gloss = 1.25f - 0.5f * (y / float(h > 1 ? h - 1 : 1));
            float f = gloss;
            bool topLeft = (x < bevel || y < bevel);
            bool bottomRight = (x >= w - bevel || y >= h - bevel);
            if (topLeft && !bottomRight)
                f *= 1.4f;
            else if (bottomRight)
                f *= 0.5f;

            SDL_Color c = shade(base, f);
            Uint8 a = cut ? 0 : 255;
            px[y * w + x] = SDL_MapRGBA(fmt, c.r, c.g, c.b, a);
        }
    }
    return finish(ren, surf);
}

SDL_Texture *makeBarTexture(SDL_Renderer *ren, int w, int h, SDL_Color base)
{
    SDL_Surface *surf = newSurface(w, h);
    Uint32 *px = static_cast<Uint32 *>(surf->pixels);
    const SDL_PixelFormat *fmt = surf->format;
    float capRadius = h / 2.0f;

    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            float alpha = 255.0f;
            if (x < capRadius)
            {
                float dx = x - capRadius, dy = (y + 0.5f) - capRadius;
                float d = std::sqrt(dx * dx + dy * dy);
                if (d > capRadius)
                    alpha = 0.0f;
            }
            else if (x >= w - capRadius)
            {
                float dx = x - (w - 1 - capRadius), dy = (y + 0.5f) - capRadius;
                float d = std::sqrt(dx * dx + dy * dy);
                if (d > capRadius)
                    alpha = 0.0f;
            }

            float ny = y / float(h > 1 ? h - 1 : 1);
            // Bright highlight stripe near the top, soft shadow toward the bottom.
            float f = 1.5f - 0.9f * ny;
            if (ny < 0.15f)
                f = 1.6f;
            SDL_Color c = shade(base, f);
            px[y * w + x] = SDL_MapRGBA(fmt, c.r, c.g, c.b, clamp8(alpha));
        }
    }
    return finish(ren, surf);
}

SDL_Texture *makeSphereTexture(SDL_Renderer *ren, int diameter, SDL_Color base)
{
    SDL_Surface *surf = newSurface(diameter, diameter);
    Uint32 *px = static_cast<Uint32 *>(surf->pixels);
    const SDL_PixelFormat *fmt = surf->format;
    float r = diameter / 2.0f;
    float cx = r, cy = r;
    float hlx = cx - r * 0.35f, hly = cy - r * 0.35f; // specular highlight offset toward upper-left

    for (int y = 0; y < diameter; y++)
    {
        for (int x = 0; x < diameter; x++)
        {
            float dx = x + 0.5f - cx, dy = y + 0.5f - cy;
            float dist = std::sqrt(dx * dx + dy * dy);
            if (dist > r + 0.5f)
            {
                px[y * diameter + x] = 0;
                continue;
            }

            float distToHighlight = std::sqrt((x + 0.5f - hlx) * (x + 0.5f - hlx) + (y + 0.5f - hly) * (y + 0.5f - hly));
            float highlight = std::max(0.0f, 1.0f - distToHighlight / (r * 0.85f));
            float rim = std::max(0.0f, (dist / r - 0.7f) / 0.3f);
            float f = 0.6f + 0.9f * highlight - 0.35f * rim;

            SDL_Color c = shade(base, f);
            float alpha = 255.0f - std::max(0.0f, dist - (r - 1.0f)) * 255.0f;
            px[y * diameter + x] = SDL_MapRGBA(fmt, c.r, c.g, c.b, clamp8(alpha));
        }
    }
    return finish(ren, surf);
}

} // namespace ProceduralTextures
