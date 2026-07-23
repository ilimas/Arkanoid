#ifndef GL_INTEROP_H
#define GL_INTEROP_H

#include <SDL.h>

// Thin, dependency-free OpenGL 2.1/GLSL-120 loader used to render a handful of
// full-screen shader effects (nebula/starfield background, the black hole,
// the ball's glow) into their own dedicated SDL render-target textures.
//
// SDL_render already owns a real GL context when it's using the "opengl"
// render driver (Game::run() asks for that driver explicitly) - we only ever
// borrow it to run a tiny shader pass into an offscreen SDL_Texture created
// with SDL_TEXTUREACCESS_TARGET. Because SDL_RenderCopy later blits that
// texture like any other, none of the interop risk of mixing raw GL draws
// with SDL_render's own draw calls applies: our GL calls never touch the
// window's default target, only textures we fully own.
//
// Every effect that uses this must treat GLInterop::available() as
// authoritative and keep its pre-existing CPU-baked-texture code path as a
// fallback: shader compilation/linking can fail for reasons impossible to
// predict from source alone (driver GLSL version, WSLg/Mesa quirks, etc.), and
// degrading to the old look is much better than a blank/broken frame.
namespace GLInterop
{
bool init(SDL_Renderer *renderer);
bool available();

// Compiles+links a program from a vertex/fragment GLSL source pair. Returns 0
// on failure (errors go to stderr); caller keeps using its fallback path.
unsigned int compileProgram(const char *vsSrc, const char *fsSrc);
void destroyProgram(unsigned int program);

int getUniformLocation(unsigned int program, const char *name);
void setUniform1f(int loc, float v);
void setUniform2f(int loc, float x, float y);
void setUniform3f(int loc, float x, float y, float z);
void setUniform1i(int loc, int v);
void setUniform4fv(int loc, int count, const float *values);

// Matches the GL viewport to the render-target texture's pixel size. Call
// this right after SDL_SetRenderTarget(renderer, targetTexture) and before
// drawFullscreenQuad, since our draw bypasses SDL_render's own viewport setup.
void setViewport(int w, int h);

// Binds `program` and draws the shared unit quad (2 triangles covering the
// whole current render target) with position/uv attribs bound by name.
void drawFullscreenQuad(unsigned int program, const char *posAttrib, const char *uvAttrib);
} // namespace GLInterop

#endif
