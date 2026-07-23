#ifndef PROCEDURAL_TEXTURES_H
#define PROCEDURAL_TEXTURES_H

#include "GLRenderer.h"
#include "Types.h"

// Generates small shaded sprites once at load time (gradient + bevel/specular
// lighting baked into the pixels), so game objects read as glossy 3D shapes
// instead of flat solid-color blocks, without needing external art.
namespace ProceduralTextures
{
// Beveled, glossy rounded-rect "candy" block: lighter top/left edge, darker
// bottom/right edge, subtle top-to-bottom gloss falloff, rounded corners.
GLRenderer::Texture makeBrickTexture(GLRenderer &gl, int w, int h, Color base);

// Glossy rounded capsule/pill bar with a bright top highlight - used for the paddle.
GLRenderer::Texture makeBarTexture(GLRenderer &gl, int w, int h, Color base);

// Glossy shaded sphere with an offset specular highlight - used for the ball.
GLRenderer::Texture makeSphereTexture(GLRenderer &gl, int diameter, Color base);

// Vertical dark navy-to-purple gradient with a soft central glow, used as the
// base wash for the procedural starfield background.
GLRenderer::Texture makeNebulaTexture(GLRenderer &gl, int w, int h);

// Horizontal streak: transparent/thin tail on the left fading into a bright,
// whitened glowing head on the right. Rotate (pivot at the right edge) to
// point the head along a comet's direction of travel.
GLRenderer::Texture makeCometTexture(GLRenderer &gl, int length, int thickness, Color color);

// Solid event-horizon core: a black disc with a warm photon-ring glow right at
// its edge, fully transparent beyond that - meant to be drawn static on top of
// the (rotating) accretion disk texture.
GLRenderer::Texture makeBlackHoleCoreTexture(GLRenderer &gl, int diameter);

// Swirling accretion-disk ring: spiral arms blending hot orange near the inner
// edge into cool purple/blue near the outer edge, transparent at the center
// and past the outer edge. Rotate continuously around its center for an
// animated swirl, without needing a loaded GIF.
GLRenderer::Texture makeBlackHoleDiskTexture(GLRenderer &gl, int diameter);
} // namespace ProceduralTextures

#endif
