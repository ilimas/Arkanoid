#ifndef GAME_H
#define GAME_H

#include "Ball.h"
#include "BlackHole.h"
#include "Bricks.h"
#include "EventManager.h"
#include "FrontendManager.h"
#include "GLRenderer.h"
#include "Paddel.h"
#include "PlayerDatabase.h"
#include "Types.h"
#include <chrono>
#include <memory>

struct GLFWwindow;

class Game
{
  public:
    Game();
    int run();

  private:
    void cleanup();
    void limitFrameRate();
    void applyResolution(int w, int h);

    // gl is declared first (destroyed last, in reverse declaration order)
    // since every other member below owns GL textures whose destructors call
    // back into it - none of them may outlive the GL context they were made
    // with. ball2 is a Game member (not a run()-local, like it used to be
    // pre-GLRenderer) for the same reason: a local would be destroyed after
    // cleanup() already tore the context down.
    std::unique_ptr<GLRenderer> gl;
    std::unique_ptr<FrontendManager> frontend;
    std::unique_ptr<Paddel> paddel;
    std::unique_ptr<BlockField> bricksField;
    std::unique_ptr<Ball> ball;
    std::unique_ptr<Ball> ball2;
    std::unique_ptr<BlackHole> blackHole;
    EventManager eventManager;
    PlayerDatabase playerDb;
    Rect bounds;
    GLFWwindow *window;
    std::chrono::steady_clock::time_point lastFrameTime_;
};

#endif
