#ifndef PROCEDURAL_TEXTURES_H
#define PROCEDURAL_TEXTURES_H

#include <SDL.h>

// Generates small shaded sprites once at load time (gradient + bevel/specular
// lighting baked into the pixels), so game objects read as glossy 3D shapes
// instead of flat SDL_RenderFillRect blocks, without needing external art.
namespace ProceduralTextures
{
// Beveled, glossy rounded-rect "candy" block: lighter top/left edge, darker
// bottom/right edge, subtle top-to-bottom gloss falloff, rounded corners.
SDL_Texture *makeBrickTexture(SDL_Renderer *ren, int w, int h, SDL_Color base);

// Glossy rounded capsule/pill bar with a bright top highlight - used for the paddle.
SDL_Texture *makeBarTexture(SDL_Renderer *ren, int w, int h, SDL_Color base);

// Glossy shaded sphere with an offset specular highlight - used for the ball.
SDL_Texture *makeSphereTexture(SDL_Renderer *ren, int diameter, SDL_Color base);
} // namespace ProceduralTextures

#endif
