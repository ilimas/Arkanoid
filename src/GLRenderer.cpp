#include "GLRenderer.h"
#include "UILayout.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <stdexcept>

// Minimal, self-contained GL 3.3 core subset: types, constants and
// function-pointer typedefs are declared by hand instead of pulling in
// <GL/gl.h>/<GL/glext.h>/GLEW/GLAD, loaded via glfwGetProcAddress. Zero extra
// link/header dependency beyond GLFW.
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

constexpr GLenum GL_COLOR_BUFFER_BIT_ = 0x00004000;
constexpr GLenum GL_TRIANGLES_ = 0x0004;
constexpr GLenum GL_ARRAY_BUFFER_ = 0x8892;
constexpr GLenum GL_STATIC_DRAW_ = 0x88E4;
constexpr GLenum GL_FLOAT_ = 0x1406;
constexpr GLboolean GL_FALSE_ = 0;
constexpr GLenum GL_VERTEX_SHADER_ = 0x8B31;
constexpr GLenum GL_FRAGMENT_SHADER_ = 0x8B30;
constexpr GLenum GL_COMPILE_STATUS_ = 0x8B81;
constexpr GLenum GL_LINK_STATUS_ = 0x8B82;
constexpr GLenum GL_BLEND_ = 0x0BE2;
constexpr GLenum GL_SRC_ALPHA_ = 0x0302;
constexpr GLenum GL_ONE_MINUS_SRC_ALPHA_ = 0x0303;
constexpr GLenum GL_SCISSOR_TEST_ = 0x0C11;
constexpr GLenum GL_TEXTURE_2D_ = 0x0DE1;
constexpr GLenum GL_TEXTURE0_ = 0x84C0;
constexpr GLenum GL_RGBA_ = 0x1908;
constexpr GLenum GL_UNSIGNED_BYTE_ = 0x1401;
constexpr GLenum GL_TEXTURE_MIN_FILTER_ = 0x2801;
constexpr GLenum GL_TEXTURE_MAG_FILTER_ = 0x2800;
constexpr GLenum GL_LINEAR_ = 0x2601;
constexpr GLenum GL_TEXTURE_WRAP_S_ = 0x2802;
constexpr GLenum GL_TEXTURE_WRAP_T_ = 0x2803;
constexpr GLenum GL_CLAMP_TO_EDGE_ = 0x812F;

using PFNGLGENBUFFERS = void (*)(GLsizei, GLuint *);
using PFNGLBINDBUFFER = void (*)(GLenum, GLuint);
using PFNGLBUFFERDATA = void (*)(GLenum, GLsizeiptr, const void *, GLenum);
using PFNGLGENVERTEXARRAYS = void (*)(GLsizei, GLuint *);
using PFNGLBINDVERTEXARRAY = void (*)(GLuint);
using PFNGLVERTEXATTRIBPOINTER = void (*)(GLuint, GLint, GLenum, GLboolean, GLsizei, const void *);
using PFNGLENABLEVERTEXATTRIBARRAY = void (*)(GLuint);
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
using PFNGLUNIFORM4F = void (*)(GLint, GLfloat, GLfloat, GLfloat, GLfloat);
using PFNGLUNIFORM1I = void (*)(GLint, GLint);
using PFNGLVIEWPORT = void (*)(GLint, GLint, GLsizei, GLsizei);
using PFNGLSCISSOR = void (*)(GLint, GLint, GLsizei, GLsizei);
using PFNGLDRAWARRAYS = void (*)(GLenum, GLint, GLsizei);
using PFNGLCLEARCOLOR = void (*)(GLfloat, GLfloat, GLfloat, GLfloat);
using PFNGLCLEAR = void (*)(GLbitfield);
using PFNGLENABLE = void (*)(GLenum);
using PFNGLDISABLE = void (*)(GLenum);
using PFNGLBLENDFUNC = void (*)(GLenum, GLenum);
using PFNGLGENTEXTURES = void (*)(GLsizei, GLuint *);
using PFNGLBINDTEXTURE = void (*)(GLenum, GLuint);
using PFNGLTEXIMAGE2D = void (*)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const void *);
using PFNGLTEXPARAMETERI = void (*)(GLenum, GLenum, GLint);
using PFNGLDELETETEXTURES = void (*)(GLsizei, const GLuint *);
using PFNGLACTIVETEXTURE = void (*)(GLenum);

PFNGLGENBUFFERS pGenBuffers;
PFNGLBINDBUFFER pBindBuffer;
PFNGLBUFFERDATA pBufferData;
PFNGLGENVERTEXARRAYS pGenVertexArrays;
PFNGLBINDVERTEXARRAY pBindVertexArray;
PFNGLVERTEXATTRIBPOINTER pVertexAttribPointer;
PFNGLENABLEVERTEXATTRIBARRAY pEnableVertexAttribArray;
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
PFNGLUNIFORM4F pUniform4f;
PFNGLUNIFORM1I pUniform1i;
PFNGLVIEWPORT pViewport;
PFNGLSCISSOR pScissor;
PFNGLDRAWARRAYS pDrawArrays;
PFNGLCLEARCOLOR pClearColor;
PFNGLCLEAR pClear;
PFNGLENABLE pEnable;
PFNGLDISABLE pDisable;
PFNGLBLENDFUNC pBlendFunc;
PFNGLGENTEXTURES pGenTextures;
PFNGLBINDTEXTURE pBindTexture;
PFNGLTEXIMAGE2D pTexImage2D;
PFNGLTEXPARAMETERI pTexParameteri;
PFNGLDELETETEXTURES pDeleteTextures;
PFNGLACTIVETEXTURE pActiveTexture;

template <typename T> bool load(T &fn, const char *name)
{
    fn = reinterpret_cast<T>(glfwGetProcAddress(name));
    return fn != nullptr;
}

// Unit quad (2 triangles, corners 0..1) - aUnit doubles as the UV coordinate
// (both go top-left -> top-right -> bottom-left -> ... in the same order),
// see the vertex shader for how position/rotation/pivot are derived from it.
constexpr float kQuadVerts[] = {
    0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, //
    0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, //
};

const char *kVertexShader = R"GLSL(
#version 330 core
layout(location = 0) in vec2 aUnit;
uniform vec2 uLogicalSize;
uniform vec4 uDstRect;   // x, y, w, h (logical px, top-left origin, y-down)
uniform vec2 uPivotUnit; // 0..1 within dst rect
uniform float uAngle;    // radians, clockwise
out vec2 vUV;

void main()
{
    vec2 pivotPx = uDstRect.xy + uPivotUnit * uDstRect.zw;
    vec2 local = (aUnit - uPivotUnit) * uDstRect.zw;
    float s = sin(uAngle);
    float c = cos(uAngle);
    vec2 rotated = vec2(local.x * c - local.y * s, local.x * s + local.y * c);
    vec2 pixelPos = pivotPx + rotated;
    vec2 ndc = (pixelPos / uLogicalSize) * 2.0 - 1.0;
    ndc.y = -ndc.y;
    vUV = aUnit;
    gl_Position = vec4(ndc, 0.0, 1.0);
}
)GLSL";

const char *kFragmentShader = R"GLSL(
#version 330 core
in vec2 vUV;
out vec4 FragColor;
uniform sampler2D uTex;
uniform vec4 uColor;
uniform int uUseTexture;

void main()
{
    if (uUseTexture != 0)
        FragColor = texture(uTex, vUV) * uColor;
    else
        FragColor = uColor;
}
)GLSL";

GLuint compileShader(GLenum type, const char *src)
{
    GLuint shader = pCreateShader(type);
    pShaderSource(shader, 1, &src, nullptr);
    pCompileShader(shader);
    GLint status = 0;
    pGetShaderiv(shader, GL_COMPILE_STATUS_, &status);
    if (!status)
    {
        char log[2048];
        pGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        std::fprintf(stderr, "GLRenderer: shader compile error: %s\n", log);
        pDeleteShader(shader);
        return 0;
    }
    return shader;
}
} // namespace

GLRenderer::GLRenderer(GLFWwindow *window)
{
    glfwMakeContextCurrent(window);

    bool ok = true;
    ok &= load(pGenBuffers, "glGenBuffers");
    ok &= load(pBindBuffer, "glBindBuffer");
    ok &= load(pBufferData, "glBufferData");
    ok &= load(pGenVertexArrays, "glGenVertexArrays");
    ok &= load(pBindVertexArray, "glBindVertexArray");
    ok &= load(pVertexAttribPointer, "glVertexAttribPointer");
    ok &= load(pEnableVertexAttribArray, "glEnableVertexAttribArray");
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
    ok &= load(pUniform4f, "glUniform4f");
    ok &= load(pUniform1i, "glUniform1i");
    ok &= load(pViewport, "glViewport");
    ok &= load(pScissor, "glScissor");
    ok &= load(pDrawArrays, "glDrawArrays");
    ok &= load(pClearColor, "glClearColor");
    ok &= load(pClear, "glClear");
    ok &= load(pEnable, "glEnable");
    ok &= load(pDisable, "glDisable");
    ok &= load(pBlendFunc, "glBlendFunc");
    ok &= load(pGenTextures, "glGenTextures");
    ok &= load(pBindTexture, "glBindTexture");
    ok &= load(pTexImage2D, "glTexImage2D");
    ok &= load(pTexParameteri, "glTexParameteri");
    ok &= load(pDeleteTextures, "glDeleteTextures");
    ok &= load(pActiveTexture, "glActiveTexture");

    if (!ok)
        throw std::runtime_error("GLRenderer: failed to resolve one or more required GL entry points");

    GLuint vs = compileShader(GL_VERTEX_SHADER_, kVertexShader);
    GLuint fs = vs ? compileShader(GL_FRAGMENT_SHADER_, kFragmentShader) : 0;
    if (!vs || !fs)
        throw std::runtime_error("GLRenderer: core shader failed to compile");

    program_ = pCreateProgram();
    pAttachShader(program_, vs);
    pAttachShader(program_, fs);
    pLinkProgram(program_);
    pDeleteShader(vs);
    pDeleteShader(fs);
    GLint linked = 0;
    pGetProgramiv(program_, GL_LINK_STATUS_, &linked);
    if (!linked)
    {
        char log[2048];
        pGetProgramInfoLog(program_, sizeof(log), nullptr, log);
        std::fprintf(stderr, "GLRenderer: program link error: %s\n", log);
        throw std::runtime_error("GLRenderer: core shader failed to link");
    }

    pGenVertexArrays(1, &vao_);
    pBindVertexArray(vao_);
    pGenBuffers(1, &vbo_);
    pBindBuffer(GL_ARRAY_BUFFER_, vbo_);
    pBufferData(GL_ARRAY_BUFFER_, (GLsizeiptr)sizeof(kQuadVerts), kQuadVerts, GL_STATIC_DRAW_);
    pVertexAttribPointer(0, 2, GL_FLOAT_, GL_FALSE_, 2 * sizeof(float), (const void *)0);
    pEnableVertexAttribArray(0);
    pBindVertexArray(0);

    pEnable(GL_BLEND_);
    pBlendFunc(GL_SRC_ALPHA_, GL_ONE_MINUS_SRC_ALPHA_);

    int fbw = 0, fbh = 0;
    glfwGetFramebufferSize(window, &fbw, &fbh);
    resize(fbw, fbh);
}

GLRenderer::~GLRenderer()
{
    if (program_)
        pDeleteProgram(program_);
}

void GLRenderer::resize(int windowW, int windowH)
{
    windowW_ = windowW;
    windowH_ = windowH;

    const float logicalW = (float)UILayout::ScreenW;
    const float logicalH = (float)UILayout::ScreenH;
    const float windowAspect = windowW / (float)windowH;
    const float logicalAspect = logicalW / logicalH;

    if (windowAspect > logicalAspect)
    {
        vpH_ = windowH;
        vpW_ = (int)(windowH * logicalAspect);
        vpX_ = (windowW - vpW_) / 2;
        vpY_ = 0;
    }
    else
    {
        vpW_ = windowW;
        vpH_ = (int)(windowW / logicalAspect);
        vpX_ = 0;
        vpY_ = (windowH - vpH_) / 2;
    }
}

void GLRenderer::beginFrame()
{
    pDisable(GL_SCISSOR_TEST_);
    pViewport(0, 0, windowW_, windowH_);
    pClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    pClear(GL_COLOR_BUFFER_BIT_);

    int glY = windowH_ - vpY_ - vpH_;
    pViewport(vpX_, glY, vpW_, vpH_);
    pScissor(vpX_, glY, vpW_, vpH_);
    pEnable(GL_SCISSOR_TEST_);
}

void GLRenderer::present(GLFWwindow *window) { glfwSwapBuffers(window); }

GLRenderer::Texture GLRenderer::createTextureFromPixels(const unsigned char *rgba, int w, int h)
{
    if (!rgba || w <= 0 || h <= 0)
        return {};

    Texture tex;
    tex.w = w;
    tex.h = h;
    pGenTextures(1, &tex.id);
    pBindTexture(GL_TEXTURE_2D_, tex.id);
    pTexParameteri(GL_TEXTURE_2D_, GL_TEXTURE_MIN_FILTER_, GL_LINEAR_);
    pTexParameteri(GL_TEXTURE_2D_, GL_TEXTURE_MAG_FILTER_, GL_LINEAR_);
    pTexParameteri(GL_TEXTURE_2D_, GL_TEXTURE_WRAP_S_, GL_CLAMP_TO_EDGE_);
    pTexParameteri(GL_TEXTURE_2D_, GL_TEXTURE_WRAP_T_, GL_CLAMP_TO_EDGE_);
    pTexImage2D(GL_TEXTURE_2D_, 0, (int)GL_RGBA_, w, h, 0, GL_RGBA_, GL_UNSIGNED_BYTE_, rgba);
    pBindTexture(GL_TEXTURE_2D_, 0);
    return tex;
}

void GLRenderer::destroyTexture(Texture &tex)
{
    if (tex.id)
        pDeleteTextures(1, &tex.id);
    tex = Texture{};
}

void GLRenderer::drawTexture(const Texture &tex, Rect dst, Color tint, double angleDeg, float pivotUnitX,
                              float pivotUnitY)
{
    if (!tex.valid())
        return;

    pUseProgram(program_);
    pBindVertexArray(vao_);
    pActiveTexture(GL_TEXTURE0_);
    pBindTexture(GL_TEXTURE_2D_, tex.id);

    pUniform2f(pGetUniformLocation(program_, "uLogicalSize"), (float)UILayout::ScreenW, (float)UILayout::ScreenH);
    pUniform4f(pGetUniformLocation(program_, "uDstRect"), (float)dst.x, (float)dst.y, (float)dst.w, (float)dst.h);
    pUniform2f(pGetUniformLocation(program_, "uPivotUnit"), pivotUnitX, pivotUnitY);
    pUniform1f(pGetUniformLocation(program_, "uAngle"), (float)(angleDeg * 3.14159265358979323846 / 180.0));
    pUniform4f(pGetUniformLocation(program_, "uColor"), tint.r / 255.0f, tint.g / 255.0f, tint.b / 255.0f,
               tint.a / 255.0f);
    pUniform1i(pGetUniformLocation(program_, "uUseTexture"), 1);
    pUniform1i(pGetUniformLocation(program_, "uTex"), 0);

    pDrawArrays(GL_TRIANGLES_, 0, 6);

    pBindVertexArray(0);
    pUseProgram(0);
}

void GLRenderer::drawColor(Rect dst, Color color)
{
    pUseProgram(program_);
    pBindVertexArray(vao_);

    pUniform2f(pGetUniformLocation(program_, "uLogicalSize"), (float)UILayout::ScreenW, (float)UILayout::ScreenH);
    pUniform4f(pGetUniformLocation(program_, "uDstRect"), (float)dst.x, (float)dst.y, (float)dst.w, (float)dst.h);
    pUniform2f(pGetUniformLocation(program_, "uPivotUnit"), 0.5f, 0.5f);
    pUniform1f(pGetUniformLocation(program_, "uAngle"), 0.0f);
    pUniform4f(pGetUniformLocation(program_, "uColor"), color.r / 255.0f, color.g / 255.0f, color.b / 255.0f,
               color.a / 255.0f);
    pUniform1i(pGetUniformLocation(program_, "uUseTexture"), 0);

    pDrawArrays(GL_TRIANGLES_, 0, 6);

    pBindVertexArray(0);
    pUseProgram(0);
}

void GLRenderer::drawColorOutline(Rect dst, Color color, int thickness)
{
    int t = std::max(1, thickness);
    drawColor({dst.x, dst.y, dst.w, t}, color);
    drawColor({dst.x, dst.y + dst.h - t, dst.w, t}, color);
    drawColor({dst.x, dst.y, t, dst.h}, color);
    drawColor({dst.x + dst.w - t, dst.y, t, dst.h}, color);
}

Point GLRenderer::windowToLogical(int rawX, int rawY) const
{
    int lx = (int)((rawX - vpX_) * (float)UILayout::ScreenW / (float)vpW_);
    int ly = (int)((rawY - vpY_) * (float)UILayout::ScreenH / (float)vpH_);
    return Point{lx, ly};
}
