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

// Vertical dark navy-to-purple gradient with a soft central glow, used as the
// base wash for the procedural starfield background.
SDL_Texture *makeNebulaTexture(SDL_Renderer *ren, int w, int h);

// Horizontal streak: transparent/thin tail on the left fading into a bright,
// whitened glowing head on the right. Rotate with SDL_RenderCopyEx (pivot at
// the right edge) to point the head along a comet's direction of travel.
SDL_Texture *makeCometTexture(SDL_Renderer *ren, int length, int thickness, SDL_Color color);
} // namespace ProceduralTextures

#endif
