#ifndef GL_RENDERER_H
#define GL_RENDERER_H

#include "Types.h"

struct GLFWwindow; // opaque; avoids pulling <GLFW/glfw3.h> (and system GL headers) into every includer

// Full replacement for SDL_Renderer/SDL_render: owns the GL context on the
// game window (created by GLFW in Game.cpp) and every draw call in the game
// goes through here.
//
// The game always renders at a fixed logical resolution (UILayout::ScreenW x
// ScreenH). GLRenderer letterboxes/pillarboxes that into whatever the actual
// window size is (so the Settings screen's resolution presets, up to 4K,
// keep working exactly like they did under SDL_RenderSetLogicalSize).
class GLRenderer
{
  public:
    struct Texture
    {
        unsigned int id{0};
        int w{0};
        int h{0};
        bool valid() const { return id != 0; }
    };

    explicit GLRenderer(GLFWwindow *window);
    ~GLRenderer();
    GLRenderer(const GLRenderer &) = delete;
    GLRenderer &operator=(const GLRenderer &) = delete;

    // Call whenever the window's actual framebuffer size changes (init, and
    // after every window resize from the Settings screen) to recompute the
    // letterboxed viewport.
    void resize(int windowW, int windowH);

    void beginFrame();
    void present(GLFWwindow *window);

    // Uploads tightly-packed RGBA8 pixels (row 0 = top of the image) as a GL
    // texture. Returns an empty (id==0) Texture on failure.
    Texture createTextureFromPixels(const unsigned char *rgba, int w, int h);
    void destroyTexture(Texture &tex);

    // dst is in logical pixels (UILayout space), top-left origin, y-down.
    // pivotUnitX/Y (0..1, default 0.5/0.5 = center) is where `angleDeg`
    // (clockwise) rotates around, fixed at dst's (x + pivotUnitX*w, y +
    // pivotUnitY*h).
    void drawTexture(const Texture &tex, Rect dst, Color tint = Color{255, 255, 255, 255}, double angleDeg = 0.0,
                      float pivotUnitX = 0.5f, float pivotUnitY = 0.5f);
    void drawColor(Rect dst, Color color);
    void drawColorOutline(Rect dst, Color color, int thickness = 1);

    // Maps a raw window pixel coordinate (as returned by glfwGetCursorPos)
    // into logical (UILayout) space, accounting for the letterbox offset.
    Point windowToLogical(int rawX, int rawY) const;

    int viewportX() const { return vpX_; }
    int viewportY() const { return vpY_; }
    int viewportW() const { return vpW_; }
    int viewportH() const { return vpH_; }

  private:
    unsigned int program_{0};
    unsigned int vbo_{0};
    unsigned int vao_{0};
    int windowW_{0}, windowH_{0};
    int vpX_{0}, vpY_{0}, vpW_{0}, vpH_{0};
};

#endif
