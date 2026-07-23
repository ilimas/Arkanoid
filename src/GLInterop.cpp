#include "GLInterop.h"
#include <cstddef>
#include <cstdio>
#include <cstring>

// Minimal, self-contained GL 2.1/GLSL-120 subset: types, constants and
// function-pointer typedefs are declared by hand instead of pulling in
// <GL/gl.h>/<GL/glext.h>, so this has zero extra link/header dependency
// beyond SDL2 (which already gives us SDL_GL_GetProcAddress).
namespace
{
using GLenum = unsigned int;
using GLboolean = unsigned char;
using GLint = int;
using GLsizei = int;
using GLuint = unsigned int;
using GLfloat = float;
using GLchar = char;
using GLbitfield = unsigned int;
using GLsizeiptr = ptrdiff_t;

constexpr GLenum GL_VERTEX_SHADER = 0x8B31;
constexpr GLenum GL_FRAGMENT_SHADER = 0x8B30;
constexpr GLenum GL_COMPILE_STATUS = 0x8B81;
constexpr GLenum GL_LINK_STATUS = 0x8B82;
constexpr GLenum GL_ARRAY_BUFFER = 0x8892;
constexpr GLenum GL_STATIC_DRAW = 0x88E4;
constexpr GLenum GL_FLOAT = 0x1406;
constexpr GLboolean GL_FALSE_ = 0;
constexpr GLenum GL_TRIANGLES = 0x0004;
constexpr GLenum GL_BLEND = 0x0BE2;
constexpr GLbitfield GL_COLOR_BUFFER_BIT = 0x00004000;

using PFNGLGENBUFFERS = void (*)(GLsizei, GLuint *);
using PFNGLBINDBUFFER = void (*)(GLenum, GLuint);
using PFNGLBUFFERDATA = void (*)(GLenum, GLsizeiptr, const void *, GLenum);
using PFNGLDELETEBUFFERS = void (*)(GLsizei, const GLuint *);
using PFNGLVERTEXATTRIBPOINTER = void (*)(GLuint, GLint, GLenum, GLboolean, GLsizei, const void *);
using PFNGLENABLEVERTEXATTRIBARRAY = void (*)(GLuint);
using PFNGLDISABLEVERTEXATTRIBARRAY = void (*)(GLuint);
using PFNGLCREATESHADER = GLuint (*)(GLenum);
using PFNGLSHADERSOURCE = void (*)(GLuint, GLsizei, const GLchar *const *, const GLint *);
using PFNGLCOMPILESHADER = void (*)(GLuint);
using PFNGLGETSHADERIV = void (*)(GLuint, GLenum, GLint *);
using PFNGLGETSHADERINFOLOG = void (*)(GLuint, GLsizei, GLsizei *, GLchar *);
using PFNGLDELETESHADER = void (*)(GLuint);
using PFNGLCREATEPROGRAM = GLuint (*)();
using PFNGLATTACHSHADER = void (*)(GLuint, GLuint);
using PFNGLLINKPROGRAM = void (*)(GLuint);
using PFNGLGETPROGRAMIV = void (*)(GLuint, GLenum, GLint *);
using PFNGLGETPROGRAMINFOLOG = void (*)(GLuint, GLsizei, GLsizei *, GLchar *);
using PFNGLDELETEPROGRAM = void (*)(GLuint);
using PFNGLUSEPROGRAM = void (*)(GLuint);
using PFNGLGETATTRIBLOCATION = GLint (*)(GLuint, const GLchar *);
using PFNGLGETUNIFORMLOCATION = GLint (*)(GLuint, const GLchar *);
using PFNGLUNIFORM1F = void (*)(GLint, GLfloat);
using PFNGLUNIFORM2F = void (*)(GLint, GLfloat, GLfloat);
using PFNGLUNIFORM3F = void (*)(GLint, GLfloat, GLfloat, GLfloat);
using PFNGLUNIFORM1I = void (*)(GLint, GLint);
using PFNGLUNIFORM4FV = void (*)(GLint, GLsizei, const GLfloat *);
using PFNGLVIEWPORT = void (*)(GLint, GLint, GLsizei, GLsizei);
using PFNGLDRAWARRAYS = void (*)(GLenum, GLint, GLsizei);
using PFNGLCLEARCOLOR = void (*)(GLfloat, GLfloat, GLfloat, GLfloat);
using PFNGLCLEAR = void (*)(GLbitfield);
using PFNGLENABLE = void (*)(GLenum);
using PFNGLDISABLE = void (*)(GLenum);
using PFNGLGETERROR = GLenum (*)();

PFNGLGENBUFFERS pGenBuffers;
PFNGLBINDBUFFER pBindBuffer;
PFNGLBUFFERDATA pBufferData;
PFNGLDELETEBUFFERS pDeleteBuffers;
PFNGLVERTEXATTRIBPOINTER pVertexAttribPointer;
PFNGLENABLEVERTEXATTRIBARRAY pEnableVertexAttribArray;
PFNGLDISABLEVERTEXATTRIBARRAY pDisableVertexAttribArray;
PFNGLCREATESHADER pCreateShader;
PFNGLSHADERSOURCE pShaderSource;
PFNGLCOMPILESHADER pCompileShader;
PFNGLGETSHADERIV pGetShaderiv;
PFNGLGETSHADERINFOLOG pGetShaderInfoLog;
PFNGLDELETESHADER pDeleteShader;
PFNGLCREATEPROGRAM pCreateProgram;
PFNGLATTACHSHADER pAttachShader;
PFNGLLINKPROGRAM pLinkProgram;
PFNGLGETPROGRAMIV pGetProgramiv;
PFNGLGETPROGRAMINFOLOG pGetProgramInfoLog;
PFNGLDELETEPROGRAM pDeleteProgram;
PFNGLUSEPROGRAM pUseProgram;
PFNGLGETATTRIBLOCATION pGetAttribLocation;
PFNGLGETUNIFORMLOCATION pGetUniformLocation;
PFNGLUNIFORM1F pUniform1f;
PFNGLUNIFORM2F pUniform2f;
PFNGLUNIFORM3F pUniform3f;
PFNGLUNIFORM1I pUniform1i;
PFNGLUNIFORM4FV pUniform4fv;
PFNGLVIEWPORT pViewport;
PFNGLDRAWARRAYS pDrawArrays;
PFNGLCLEARCOLOR pClearColor;
PFNGLCLEAR pClear;
PFNGLENABLE pEnable;
PFNGLDISABLE pDisable;
PFNGLGETERROR pGetError;

bool g_available = false;
GLuint g_quadVBO = 0;

// Vertex Y is deliberately flipped relative to the "natural" NDC->UV mapping:
// GL rasterizes NDC y=-1 into texel row 0 of whatever's attached to the
// current FBO (window or, here, our SDL render-target texture), but SDL's own
// texture convention treats texel row 0 as the TOP of the image (that's the
// well-known reason custom-shader SDL2/GL tutorials flip V when sampling an
// SDL_Texture directly). Since we render INTO an SDL-owned target texture
// with our own raw draw instead of through SDL_render, we bake the
// equivalent flip into the quad itself so the texture SDL_RenderCopy later
// blits comes out right-side up, matching what its own fragment shaders
// (uv.y=0 => top of the image) expect.
constexpr float kQuadVerts[] = {
    // x,    y,    u,   v
    -1.0f, -1.0f, 0.0f, 0.0f,
    1.0f,  -1.0f, 1.0f, 0.0f,
    -1.0f, 1.0f,  0.0f, 1.0f,
    -1.0f, 1.0f,  0.0f, 1.0f,
    1.0f,  -1.0f, 1.0f, 0.0f,
    1.0f,  1.0f,  1.0f, 1.0f,
};

template <typename T> bool load(T &fn, const char *name)
{
    fn = reinterpret_cast<T>(SDL_GL_GetProcAddress(name));
    return fn != nullptr;
}
} // namespace

namespace GLInterop
{
bool init(SDL_Renderer *renderer)
{
    g_available = false;

    SDL_RendererInfo info;
    if (SDL_GetRendererInfo(renderer, &info) != 0 || std::strcmp(info.name, "opengl") != 0)
    {
        std::fprintf(stderr, "GLInterop: renderer is not using the opengl driver (\"%s\"), shader effects disabled\n",
                     info.name ? info.name : "?");
        return false;
    }

    bool ok = true;
    ok &= load(pGenBuffers, "glGenBuffers");
    ok &= load(pBindBuffer, "glBindBuffer");
    ok &= load(pBufferData, "glBufferData");
    ok &= load(pDeleteBuffers, "glDeleteBuffers");
    ok &= load(pVertexAttribPointer, "glVertexAttribPointer");
    ok &= load(pEnableVertexAttribArray, "glEnableVertexAttribArray");
    ok &= load(pDisableVertexAttribArray, "glDisableVertexAttribArray");
    ok &= load(pCreateShader, "glCreateShader");
    ok &= load(pShaderSource, "glShaderSource");
    ok &= load(pCompileShader, "glCompileShader");
    ok &= load(pGetShaderiv, "glGetShaderiv");
    ok &= load(pGetShaderInfoLog, "glGetShaderInfoLog");
    ok &= load(pDeleteShader, "glDeleteShader");
    ok &= load(pCreateProgram, "glCreateProgram");
    ok &= load(pAttachShader, "glAttachShader");
    ok &= load(pLinkProgram, "glLinkProgram");
    ok &= load(pGetProgramiv, "glGetProgramiv");
    ok &= load(pGetProgramInfoLog, "glGetProgramInfoLog");
    ok &= load(pDeleteProgram, "glDeleteProgram");
    ok &= load(pUseProgram, "glUseProgram");
    ok &= load(pGetAttribLocation, "glGetAttribLocation");
    ok &= load(pGetUniformLocation, "glGetUniformLocation");
    ok &= load(pUniform1f, "glUniform1f");
    ok &= load(pUniform2f, "glUniform2f");
    ok &= load(pUniform3f, "glUniform3f");
    ok &= load(pUniform1i, "glUniform1i");
    ok &= load(pUniform4fv, "glUniform4fv");
    ok &= load(pViewport, "glViewport");
    ok &= load(pDrawArrays, "glDrawArrays");
    ok &= load(pClearColor, "glClearColor");
    ok &= load(pClear, "glClear");
    ok &= load(pEnable, "glEnable");
    ok &= load(pDisable, "glDisable");
    ok &= load(pGetError, "glGetError");

    if (!ok)
    {
        std::fprintf(stderr, "GLInterop: failed to resolve one or more GL entry points, shader effects disabled\n");
        return false;
    }

    pGenBuffers(1, &g_quadVBO);
    pBindBuffer(GL_ARRAY_BUFFER, g_quadVBO);
    pBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)sizeof(kQuadVerts), kQuadVerts, GL_STATIC_DRAW);
    pBindBuffer(GL_ARRAY_BUFFER, 0);

    g_available = true;
    return true;
}

bool available() { return g_available; }

unsigned int compileProgram(const char *vsSrc, const char *fsSrc)
{
    if (!g_available)
        return 0;

    auto compile = [](GLenum type, const char *src) -> GLuint {
        GLuint shader = pCreateShader(type);
        pShaderSource(shader, 1, &src, nullptr);
        pCompileShader(shader);
        GLint status = 0;
        pGetShaderiv(shader, GL_COMPILE_STATUS, &status);
        if (!status)
        {
            char log[1024];
            pGetShaderInfoLog(shader, sizeof(log), nullptr, log);
            std::fprintf(stderr, "GLInterop: shader compile error: %s\n", log);
            pDeleteShader(shader);
            return 0;
        }
        return shader;
    };

    GLuint vs = compile(GL_VERTEX_SHADER, vsSrc);
    GLuint fs = vs ? compile(GL_FRAGMENT_SHADER, fsSrc) : 0;
    if (!vs || !fs)
    {
        if (vs)
            pDeleteShader(vs);
        if (fs)
            pDeleteShader(fs);
        return 0;
    }

    GLuint program = pCreateProgram();
    pAttachShader(program, vs);
    pAttachShader(program, fs);
    pLinkProgram(program);
    pDeleteShader(vs);
    pDeleteShader(fs);

    GLint linked = 0;
    pGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked)
    {
        char log[1024];
        pGetProgramInfoLog(program, sizeof(log), nullptr, log);
        std::fprintf(stderr, "GLInterop: program link error: %s\n", log);
        pDeleteProgram(program);
        return 0;
    }
    return program;
}

void destroyProgram(unsigned int program)
{
    if (g_available && program)
        pDeleteProgram(program);
}

int getUniformLocation(unsigned int program, const char *name)
{
    if (!g_available)
        return -1;
    return pGetUniformLocation(program, name);
}

void setUniform1f(int loc, float v)
{
    if (g_available && loc >= 0)
        pUniform1f(loc, v);
}

void setUniform2f(int loc, float x, float y)
{
    if (g_available && loc >= 0)
        pUniform2f(loc, x, y);
}

void setUniform3f(int loc, float x, float y, float z)
{
    if (g_available && loc >= 0)
        pUniform3f(loc, x, y, z);
}

void setUniform1i(int loc, int v)
{
    if (g_available && loc >= 0)
        pUniform1i(loc, v);
}

void setUniform4fv(int loc, int count, const float *values)
{
    if (g_available && loc >= 0)
        pUniform4fv(loc, count, values);
}

void setViewport(int w, int h)
{
    if (g_available)
        pViewport(0, 0, w, h);
}

void drawFullscreenQuad(unsigned int program, const char *posAttrib, const char *uvAttrib)
{
    if (!g_available)
        return;

    pUseProgram(program);
    pDisable(GL_BLEND); // write raw (unpremultiplied) RGBA; SDL_RenderCopy blends this texture later

    pBindBuffer(GL_ARRAY_BUFFER, g_quadVBO);
    GLint posLoc = pGetAttribLocation(program, posAttrib);
    GLint uvLoc = pGetAttribLocation(program, uvAttrib);
    if (posLoc >= 0)
    {
        pEnableVertexAttribArray((GLuint)posLoc);
        pVertexAttribPointer((GLuint)posLoc, 2, GL_FLOAT, GL_FALSE_, 4 * sizeof(float), (const void *)0);
    }
    if (uvLoc >= 0)
    {
        pEnableVertexAttribArray((GLuint)uvLoc);
        pVertexAttribPointer((GLuint)uvLoc, 2, GL_FLOAT, GL_FALSE_, 4 * sizeof(float),
                              (const void *)(2 * sizeof(float)));
    }

    pClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    pClear(GL_COLOR_BUFFER_BIT);
    pDrawArrays(GL_TRIANGLES, 0, 6);

    if (posLoc >= 0)
        pDisableVertexAttribArray((GLuint)posLoc);
    if (uvLoc >= 0)
        pDisableVertexAttribArray((GLuint)uvLoc);
    pBindBuffer(GL_ARRAY_BUFFER, 0);
    pUseProgram(0);
}

} // namespace GLInterop
