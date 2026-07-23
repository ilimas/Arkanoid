#include "BlackHole.h"
#include "GLInterop.h"
#include "ProceduralTextures.h"
#include <SDL_rect.h>
#include <SDL_render.h>
#include <SDL_timer.h>
#include <cmath>
#include <cstdlib>
#include <numbers>

namespace
{
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

// Event horizon + photon ring + a two-layer counter-rotating spiral accretion
// disk, all in one pass - replaces the CPU-baked texCore/texDisk textures and
// the double SDL_RenderCopyEx rotation trick below.
const char *kFragmentShader = R"GLSL(
#version 120
varying vec2 vUV;
uniform vec2 uResolution;
uniform float uTime;

void main()
{
    float r = uResolution.x * 0.5;
    vec2 center = uResolution * 0.5;
    vec2 d = vUV * uResolution - center;
    float dist = length(d);

    float coreR = r * 0.55;
    float ringR = r * 0.72;
    float diskOuter = r * 0.99;

    if (dist > diskOuter)
    {
        gl_FragColor = vec4(0.0);
        return;
    }

    if (dist <= coreR)
    {
        float f = 1.0 - 0.15 * (dist / coreR);
        gl_FragColor = vec4(vec3(6.0, 3.0, 12.0) / 255.0 * f, 1.0);
        return;
    }

    if (dist <= ringR)
    {
        float t = (dist - coreR) / (ringR - coreR);
        float glowAmt = sin(t * 3.14159265);
        gl_FragColor = vec4(vec3(255.0, 205.0, 150.0) / 255.0, glowAmt);
        return;
    }

    float t = clamp((dist - ringR) / (diskOuter - ringR), 0.0, 1.0);
    float angle = atan(d.y, d.x);
    float edge = smoothstep(ringR - 2.0, ringR + 2.0, dist) * (1.0 - smoothstep(diskOuter - 5.0, diskOuter, dist));

    float spinSpeed = 1.9;
    float spiral1 = 0.5 + 0.5 * sin(angle * 5.0 - t * 5.0 * 6.2831853 + uTime * spinSpeed);
    float spiral2 = 0.5 + 0.5 * sin(angle * 5.0 - t * 5.0 * 6.2831853 - uTime * spinSpeed * 0.6);

    vec3 hot = vec3(255.0, 150.0, 70.0) / 255.0;
    vec3 cool = vec3(130.0, 70.0, 230.0) / 255.0;
    vec3 c = mix(hot, cool, t);

    float a1 = spiral1 * (1.0 - t * 0.55) * edge;
    float a2 = spiral2 * (1.0 - t * 0.55) * edge * 0.6;
    float aTotal = clamp(a1 + a2 * (1.0 - a1), 0.0, 1.0);
    vec3 cTotal = a1 * c + a2 * (1.0 - a1) * c;

    gl_FragColor = vec4(cTotal, aTotal);
}
)GLSL";
} // namespace

BlackHole::BlackHole(SDL_Rect bounds_, SDL_Renderer *render_)
{
    bounds = bounds_;
    render = render_;
    texDisk = ProceduralTextures::makeBlackHoleDiskTexture(render, DIAMETER);
    texCore = ProceduralTextures::makeBlackHoleCoreTexture(render, DIAMETER);

    if (GLInterop::available())
    {
        shaderProgram_ = GLInterop::compileProgram(kVertexShader, kFragmentShader);
        if (shaderProgram_)
        {
            shaderTarget_ =
                SDL_CreateTexture(render, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, DIAMETER, DIAMETER);
            if (shaderTarget_)
                SDL_SetTextureBlendMode(shaderTarget_, SDL_BLENDMODE_BLEND);
            else
            {
                GLInterop::destroyProgram(shaderProgram_);
                shaderProgram_ = 0;
            }
        }
    }
}

BlackHole::~BlackHole()
{
    SDL_DestroyTexture(texDisk);
    SDL_DestroyTexture(texCore);
    if (shaderTarget_)
        SDL_DestroyTexture(shaderTarget_);
    GLInterop::destroyProgram(shaderProgram_);
}

void BlackHole::reset()
{
    diskAngle = 0.0f;
    lastMoveTime = SDL_GetTicks();
    auto randomInt = [](int min, int max) { return min + std::rand() % (max - min); };
    x = static_cast<float>(randomInt(48, bounds.w - DIAMETER));
    y = static_cast<float>(randomInt(48, bounds.h - DIAMETER));
    double angle = (std::rand() % 360) * std::numbers::pi / 180.0;
    vx = static_cast<float>(std::cos(angle) * SPEED);
    vy = static_cast<float>(std::sin(angle) * SPEED);
}

void BlackHole::draw()
{
    auto now = SDL_GetTicks();
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
            totalTime_ += dt;
            x += vx * dt;
            y += vy * dt;
            if (x < 0.0f)               { x = 0.0f;                              vx = -vx; }
            if (x > bounds.w - DIAMETER) { x = static_cast<float>(bounds.w - DIAMETER); vx = -vx; }
            if (y < 0.0f)               { y = 0.0f;                              vy = -vy; }
            if (y > bounds.h - DIAMETER) { y = static_cast<float>(bounds.h - DIAMETER); vy = -vy; }

            diskAngle += dt * DISK_ROTATION_SPEED;
            if (diskAngle >= 360.0f)
                diskAngle -= 360.0f;

            SDL_Rect dst = {(int)x, (int)y, DIAMETER, DIAMETER};

            if (GLInterop::available() && shaderProgram_ && shaderTarget_)
            {
                SDL_Texture *prevTarget = SDL_GetRenderTarget(render);
                SDL_SetRenderTarget(render, shaderTarget_);
                GLInterop::setViewport(DIAMETER, DIAMETER);
                GLInterop::setUniform2f(GLInterop::getUniformLocation(shaderProgram_, "uResolution"),
                                         (float)DIAMETER, (float)DIAMETER);
                GLInterop::setUniform1f(GLInterop::getUniformLocation(shaderProgram_, "uTime"), totalTime_);
                GLInterop::drawFullscreenQuad(shaderProgram_, "aPos", "aUV");
                SDL_SetRenderTarget(render, prevTarget);

                SDL_RenderCopy(render, shaderTarget_, nullptr, &dst);
            }
            else
            {
                SDL_Point center{DIAMETER / 2, DIAMETER / 2};
                // Two counter-rotating passes of the same spiral texture give the
                // disk a layered, parallax-like swirl instead of a flat spin.
                SDL_RenderCopyEx(render, texDisk, nullptr, &dst, diskAngle, &center, SDL_FLIP_NONE);
                SDL_RenderCopyEx(render, texDisk, nullptr, &dst, -diskAngle * 0.6, &center, SDL_FLIP_NONE);
                SDL_RenderCopy(render, texCore, nullptr, &dst);
            }
        }
    }
}
