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

SDL_Color lerpColor(SDL_Color a, SDL_Color b, float t)
{
    t = std::clamp(t, 0.0f, 1.0f);
    return SDL_Color{clamp8(a.r + (b.r - a.r) * t), clamp8(a.g + (b.g - a.g) * t), clamp8(a.b + (b.b - a.b) * t),
                      clamp8(a.a + (b.a - a.a) * t)};
}

float smoothstep(float edge0, float edge1, float x)
{
    float t = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
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

SDL_Texture *makeNebulaTexture(SDL_Renderer *ren, int w, int h)
{
    SDL_Surface *surf = newSurface(w, h);
    Uint32 *px = static_cast<Uint32 *>(surf->pixels);
    const SDL_PixelFormat *fmt = surf->format;
    SDL_Color top{8, 10, 24, 255};
    SDL_Color bottom{22, 15, 42, 255};
    float cx = w / 2.0f, cy = h * 0.4f;
    float maxDist = std::sqrt(cx * cx + cy * cy);

    for (int y = 0; y < h; y++)
    {
        float t = y / float(h > 1 ? h - 1 : 1);
        float baseR = top.r + (bottom.r - top.r) * t;
        float baseG = top.g + (bottom.g - top.g) * t;
        float baseB = top.b + (bottom.b - top.b) * t;
        for (int x = 0; x < w; x++)
        {
            float dx = x - cx, dy = y - cy;
            float dist = std::sqrt(dx * dx + dy * dy) / maxDist;
            float glow = std::max(0.0f, 1.0f - dist) * 0.2f;
            Uint8 r = clamp8(baseR + glow * 45.0f);
            Uint8 g = clamp8(baseG + glow * 38.0f);
            Uint8 b = clamp8(baseB + glow * 60.0f);
            px[y * w + x] = SDL_MapRGBA(fmt, r, g, b, 255);
        }
    }
    return finish(ren, surf);
}

SDL_Texture *makeCometTexture(SDL_Renderer *ren, int length, int thickness, SDL_Color color)
{
    SDL_Surface *surf = newSurface(length, thickness);
    Uint32 *px = static_cast<Uint32 *>(surf->pixels);
    const SDL_PixelFormat *fmt = surf->format;
    float cy = thickness / 2.0f;

    for (int y = 0; y < thickness; y++)
    {
        for (int x = 0; x < length; x++)
        {
            float nx = x / float(length > 1 ? length - 1 : 1); // 0 = tail, 1 = head
            float radius = 0.5f + (thickness * 0.5f) * std::pow(nx, 1.5f);
            float dy = std::fabs((y + 0.5f) - cy);
            if (dy > radius)
            {
                px[y * length + x] = 0;
                continue;
            }
            float edgeSoft = 1.0f - dy / radius;
            float fade = std::pow(nx, 1.6f);
            float alpha = fade * edgeSoft * 255.0f;

            float whiten = std::max(0.0f, nx - 0.75f) / 0.25f;
            Uint8 r = clamp8(color.r + whiten * (255 - color.r));
            Uint8 g = clamp8(color.g + whiten * (255 - color.g));
            Uint8 b = clamp8(color.b + whiten * (255 - color.b));
            px[y * length + x] = SDL_MapRGBA(fmt, r, g, b, clamp8(alpha));
        }
    }
    return finish(ren, surf);
}

SDL_Texture *makeBlackHoleCoreTexture(SDL_Renderer *ren, int diameter)
{
    SDL_Surface *surf = newSurface(diameter, diameter);
    Uint32 *px = static_cast<Uint32 *>(surf->pixels);
    const SDL_PixelFormat *fmt = surf->format;
    float r = diameter / 2.0f;
    float cx = r, cy = r;
    float coreR = r * 0.55f;
    float ringR = r * 0.72f;
    SDL_Color ringHot{255, 205, 150, 255};

    for (int y = 0; y < diameter; y++)
    {
        for (int x = 0; x < diameter; x++)
        {
            float dx = x + 0.5f - cx, dy = y + 0.5f - cy;
            float dist = std::sqrt(dx * dx + dy * dy);

            if (dist <= coreR)
            {
                // Near-black event horizon with a faint inward vignette for depth.
                float f = 1.0f - 0.15f * (dist / coreR);
                SDL_Color c = shade(SDL_Color{6, 3, 12, 255}, f);
                px[y * diameter + x] = SDL_MapRGBA(fmt, c.r, c.g, c.b, 255);
            }
            else if (dist <= ringR)
            {
                // Bright photon ring right at the horizon's edge, fading out both
                // toward the black core and toward the surrounding disk.
                float t = (dist - coreR) / (ringR - coreR);
                float glow = std::sin(t * (float)M_PI); // 0 at both ends, peak at t=0.5
                SDL_Color c = ringHot;
                Uint8 a = clamp8(glow * 255.0f);
                px[y * diameter + x] = SDL_MapRGBA(fmt, c.r, c.g, c.b, a);
            }
            else
            {
                px[y * diameter + x] = 0;
            }
        }
    }
    return finish(ren, surf);
}

SDL_Texture *makeBlackHoleDiskTexture(SDL_Renderer *ren, int diameter)
{
    SDL_Surface *surf = newSurface(diameter, diameter);
    Uint32 *px = static_cast<Uint32 *>(surf->pixels);
    const SDL_PixelFormat *fmt = surf->format;
    float r = diameter / 2.0f;
    float cx = r, cy = r;
    float rInner = r * 0.60f;
    float rOuter = r * 0.99f;
    constexpr float arms = 5.0f;
    constexpr float twist = 5.0f;
    SDL_Color hot{255, 150, 70, 255};
    SDL_Color cool{130, 70, 230, 255};

    for (int y = 0; y < diameter; y++)
    {
        for (int x = 0; x < diameter; x++)
        {
            float dx = x + 0.5f - cx, dy = y + 0.5f - cy;
            float dist = std::sqrt(dx * dx + dy * dy);
            if (dist < rInner - 2.0f || dist > rOuter)
            {
                px[y * diameter + x] = 0;
                continue;
            }

            float t = std::clamp((dist - rInner) / (rOuter - rInner), 0.0f, 1.0f);
            float angle = std::atan2(dy, dx);
            float spiral = 0.5f + 0.5f * std::sin(angle * arms - t * twist * 2.0f * (float)M_PI);
            float edgeFalloff = smoothstep(rInner - 2.0f, rInner + 2.0f, dist) * (1.0f - smoothstep(rOuter - 5.0f, rOuter, dist));
            float brightness = spiral * (1.0f - t * 0.55f);

            SDL_Color c = lerpColor(hot, cool, t);
            Uint8 a = clamp8(brightness * edgeFalloff * 235.0f);
            px[y * diameter + x] = SDL_MapRGBA(fmt, c.r, c.g, c.b, a);
        }
    }
    return finish(ren, surf);
}

} // namespace ProceduralTextures
