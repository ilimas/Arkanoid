#include "Starfield.h"
#include "GLInterop.h"
#include "ProceduralTextures.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace
{
float frand(float lo, float hi) { return lo + (hi - lo) * (std::rand() / (float)RAND_MAX); }

constexpr int kMaxShaderComets = 8;

const char *kVertexShader = R"GLSL(
#version 120
attribute vec2 aPos;
attribute vec2 aUV;
varying vec2 vUV;
void main()
{
    vUV = aUV;
    gl_Position = vec4(aPos, 0.0, 1.0);
}
)GLSL";

// Nebula wash + hashed/twinkling stars + comet-glow streaks, all evaluated
// per-pixel every frame - replaces the CPU-baked nebula texture plus the
// per-star SDL_RenderFillRect and per-comet SDL_RenderCopyEx calls below.
const char *kFragmentShader = R"GLSL(
#version 120
varying vec2 vUV;
uniform vec2 uResolution;
uniform float uTime;
uniform vec4 uCometPosDir[8];
uniform vec4 uCometColor[8];
uniform int uCometCount;

float hash(vec2 p)
{
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453123);
}

void main()
{
    vec2 px = vUV * uResolution;

    vec3 topC = vec3(8.0, 10.0, 24.0) / 255.0;
    vec3 botC = vec3(22.0, 15.0, 42.0) / 255.0;
    vec3 col = mix(topC, botC, vUV.y);

    vec2 center = vec2(uResolution.x * 0.5, uResolution.y * 0.4);
    float maxDist = length(uResolution);
    float dist = length(px - center) / maxDist;
    float glow = max(0.0, 1.0 - dist) * 0.2;
    col += glow * (vec3(45.0, 38.0, 60.0) / 255.0);

    vec2 cell = floor(px);
    float starRand = hash(cell);
    if (starRand > 0.9965)
    {
        float depth = hash(cell + vec2(3.7, 1.3));
        float phase = hash(cell + vec2(9.2, 4.4)) * 6.2831853;
        float speed = 0.6 + depth * 1.6;
        float twinkle = 0.55 + 0.45 * sin(uTime * speed + phase);
        float baseAlpha = (90.0 + depth * 165.0) / 255.0;
        col = mix(col, vec3(235.0, 240.0, 255.0) / 255.0, baseAlpha * twinkle);
    }

    for (int i = 0; i < 8; i++)
    {
        if (i >= uCometCount)
            break;
        vec2 p0 = uCometPosDir[i].xy;
        vec2 dir = uCometPosDir[i].zw;
        float dirLen = length(dir);
        if (dirLen < 0.0001)
            continue;
        dir /= dirLen;
        vec2 rel = px - p0;
        float t = clamp(dot(rel, dir), -150.0, 0.0);
        vec2 closest = p0 + dir * t;
        float d = length(px - closest);
        float tailFade = clamp(-t / 150.0, 0.0, 1.0);
        float glowA = smoothstep(4.0, 0.0, d) * pow(1.0 - tailFade, 1.6) * uCometColor[i].a;
        col = mix(col, uCometColor[i].rgb, glowA);
    }

    gl_FragColor = vec4(col, 1.0);
}
)GLSL";
} // namespace

Starfield::Starfield(SDL_Renderer *ren, int width, int height) : render(ren), width_(width), height_(height)
{
    nebulaTex = ProceduralTextures::makeNebulaTexture(render, width_, height_);
    cometTex = ProceduralTextures::makeCometTexture(render, 144, 8, SDL_Color{200, 220, 255, 255});

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

    if (GLInterop::available())
    {
        shaderProgram_ = GLInterop::compileProgram(kVertexShader, kFragmentShader);
        if (shaderProgram_)
        {
            shaderTarget_ = SDL_CreateTexture(render, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, width_,
                                               height_);
            if (!shaderTarget_)
            {
                GLInterop::destroyProgram(shaderProgram_);
                shaderProgram_ = 0;
            }
        }
    }
}

Starfield::~Starfield()
{
    SDL_DestroyTexture(nebulaTex);
    SDL_DestroyTexture(cometTex);
    if (shaderTarget_)
        SDL_DestroyTexture(shaderTarget_);
    GLInterop::destroyProgram(shaderProgram_);
}

void Starfield::spawnComet()
{
    static const SDL_Color palette[3] = {
        {200, 220, 255, 255},
        {255, 235, 210, 255},
        {215, 255, 235, 255},
    };

    Comet c;
    bool fromLeft = std::rand() % 2 == 0;
    float speed = frand(416.0f, 672.0f);
    // Angle in radians measured the way SDL_RenderCopyEx rotates (clockwise in
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
    totalTime_ += (float)dt;

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
    if (GLInterop::available() && shaderProgram_ && shaderTarget_)
    {
        float posDir[kMaxShaderComets * 4] = {};
        float colorA[kMaxShaderComets * 4] = {};
        int count = std::min<int>((int)comets.size(), kMaxShaderComets);
        for (int i = 0; i < count; i++)
        {
            const Comet &c = comets[i];
            posDir[i * 4 + 0] = c.x;
            posDir[i * 4 + 1] = c.y;
            posDir[i * 4 + 2] = c.vx;
            posDir[i * 4 + 3] = c.vy;
            colorA[i * 4 + 0] = c.color.r / 255.0f;
            colorA[i * 4 + 1] = c.color.g / 255.0f;
            colorA[i * 4 + 2] = c.color.b / 255.0f;
            colorA[i * 4 + 3] = 1.0f;
        }

        SDL_Texture *prevTarget = SDL_GetRenderTarget(render);
        SDL_SetRenderTarget(render, shaderTarget_);
        GLInterop::setViewport(width_, height_);
        GLInterop::setUniform2f(GLInterop::getUniformLocation(shaderProgram_, "uResolution"), (float)width_,
                                 (float)height_);
        GLInterop::setUniform1f(GLInterop::getUniformLocation(shaderProgram_, "uTime"), totalTime_);
        GLInterop::setUniform1i(GLInterop::getUniformLocation(shaderProgram_, "uCometCount"), count);
        GLInterop::setUniform4fv(GLInterop::getUniformLocation(shaderProgram_, "uCometPosDir"), kMaxShaderComets,
                                  posDir);
        GLInterop::setUniform4fv(GLInterop::getUniformLocation(shaderProgram_, "uCometColor"), kMaxShaderComets,
                                  colorA);
        GLInterop::drawFullscreenQuad(shaderProgram_, "aPos", "aUV");
        SDL_SetRenderTarget(render, prevTarget);

        SDL_RenderCopy(render, shaderTarget_, nullptr, nullptr);
        return;
    }

    SDL_RenderCopy(render, nebulaTex, nullptr, nullptr);

    for (auto &s : stars)
    {
        float twinkle = 0.55f + 0.45f * std::sin(s.twinklePhase);
        Uint8 a = (Uint8)std::clamp(s.baseAlpha * twinkle, 0.0f, 255.0f);
        SDL_SetRenderDrawColor(render, 235, 240, 255, a);
        SDL_Rect r{(int)s.x, (int)s.y, (int)std::ceil(s.size), (int)std::ceil(s.size)};
        SDL_RenderFillRect(render, &r);
    }

    int texW, texH;
    SDL_QueryTexture(cometTex, nullptr, nullptr, &texW, &texH);
    SDL_Point pivot{texW, texH / 2};
    for (auto &c : comets)
    {
        float angleDeg = std::atan2(c.vy, c.vx) * 180.0f / (float)M_PI;
        SDL_SetTextureColorMod(cometTex, c.color.r, c.color.g, c.color.b);
        SDL_Rect dst{(int)c.x - texW, (int)c.y - texH / 2, texW, texH};
        SDL_RenderCopyEx(render, cometTex, nullptr, &dst, angleDeg, &pivot, SDL_FLIP_NONE);
    }
}
