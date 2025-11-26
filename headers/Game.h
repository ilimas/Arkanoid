#ifndef GAME_H
#define GAME_H

#include "Ball.h"
#include "Bricks.h"
#include "EventManager.h"
#include "FrontendManager.h"
#include "Paddel.h"
#include <SDL_mixer.h>
#include <SDL_rect.h>
#include <SDL_render.h>
#include <memory>

class Game
{
  public:
    Game();
    int run();

  private:
    void cleanup();

    std::unique_ptr<FrontendManager> frontend;
    std::unique_ptr<Paddel> paddel;
    std::unique_ptr<BlockField> bricksField;
    std::unique_ptr<Ball> ball;
    EventManager eventManager;
    SDL_Rect bounds;
    SDL_Window *window;
    SDL_Renderer *renderer;
    Mix_Music *music;
};

#endif
