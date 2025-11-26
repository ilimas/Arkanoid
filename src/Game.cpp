#include "Game.h"
#include "Ball.h"
#include "Bricks.h"
#include "EventManager.h"
#include "FrontendManager.h"
#include "Paddel.h"
#include <SDL_mixer.h>
#include <SDL_rect.h>
#include <SDL_render.h>
#include <iostream>
#include <memory>

Game::Game() : bounds{0, 0, 640, 480},
               frontend(nullptr),
               renderer(nullptr),
               window(nullptr),
               music(nullptr)
{
}

int Game::run()
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << "\n";
        return 1;
    }
    window = SDL_CreateWindow("Shalnoi (refactor)", 20, 100, bounds.w, bounds.h, SDL_WINDOW_SHOWN);
    if (!window)
    {
        std::cerr << "CreateWindow: " << SDL_GetError() << "\n";
        return 1;
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer)
    {
        std::cerr << "CreateRenderer: " << SDL_GetError() << "\n";
        return 1;
    }
    if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) == -1)
    {
        std::cerr << "Mix_OpenAudio failed\n";
    }
    else
    {
        // music = Mix_LoadMUS(SNACKS_DIR "/music/shrek.wav");
        // if (music)
        //     Mix_PlayMusic(music, -1);
    }

    frontend = std::make_unique<FrontendManager>(renderer);
    paddel = std::make_unique<Paddel>(renderer);
    bricksField = std::make_unique<BlockField>(renderer);
    ball = std::make_unique<Ball>(bounds, renderer);
    int mx = 0, my = 0;
    int score = 0;
    uint32_t tstart = SDL_GetTicks();
    SDL_ShowCursor(SDL_ENABLE);
    eventManager.startCaptureEvents();
    while (!eventManager.getState().gameOver)
    {
        while (eventManager.getState().gameMenu)
        {
            frontend->draw_menu(eventManager.getState().menuSelectedItem.load());
            SDL_RenderPresent(renderer);
        }
        if (eventManager.getState().gameOver)
            break;
        eventManager.startCaptureEvents();
        eventManager.getState().gameStart = false;
        eventManager.getState().gamePaused = false;
        eventManager.getState().gameOver = false;
        bricksField->load_level();
        ball->setmain();
        paddel->setmain();
        score = 0;
        tstart = SDL_GetTicks();

        while (eventManager.getState().gameWelcome)
        {
            frontend->draw_background();
            paddel->draw();
            bricksField->draw();
            ball->draw();
            frontend->draw_welcome_text();
            SDL_RenderPresent(renderer);
            SDL_Delay(8);
        }

        SDL_ShowCursor(SDL_DISABLE);
        while (eventManager.getState().gameStart)
        {
            while (eventManager.getState().gamePaused)
            {
                frontend->draw_background();
                frontend->draw_pause();
                SDL_RenderPresent(renderer);
                SDL_Delay(8);
            }
            if (eventManager.getState().gameOver == true)
                break;
            SDL_GetMouseState(&mx, nullptr);
            if (mx >= 50 && mx <= 590)
            {
                paddel->setpos(mx - 50);
            }

            frontend->draw_background();
            if (ball->get_position().y >= 470.0f)
            {
                eventManager.getState().gamePreEnd = true;
                while (eventManager.getState().gamePreEnd)
                {
                    frontend->draw_end();
                    SDL_RenderPresent(renderer);
                    SDL_Delay(8);
                    frontend->draw_background();
                };
                break;
            }
            else if (!ball->out_of_bounds())
            {
                auto &blocks_vector = bricksField->getBlocksVector();
                for (int i = 0; i < bricksField->bsize(); ++i)
                {
                    auto &current_block = bricksField->retBlock(i);
                    if (auto closest_point = ball->touches(current_block); closest_point.has_value())
                    {
                        HitSide side = current_block.detectHitSide(ball->get_position(), ball->radius);
                        ball->set_destination(current_block.reflectBall(*ball));
                        // auto normal = ball->get_position() - closest_point.value();
                        // auto distance = normal.length();
                        // auto penetration = ball->radius - distance;
                        // ball->set_position(ball->get_position() + normal * penetration);
                        bricksField->minus(i);
                        if (bricksField->retvar(i) == 0)
                        {
                            bricksField->del(i);
                            score++;
                        }
                    }
                }

                if (auto closest_point = ball->touches(*paddel); closest_point.has_value())
                {
                    Vec2 closest_point_vec = closest_point.value();
                    int paddel_width = 100;
                    double paddel_center_x = paddel->retx() + paddel_width / 2.f;
                    double offset =
                        (closest_point_vec.x - paddel_center_x) / (paddel_width / 2.f);
                    if (offset < -1.f)
                        offset = -1.f;
                    if (offset > 1.f)
                        offset = 1.f;

                    Vec2 bounce_normal{offset, -1.f};
                    bounce_normal = bounce_normal.normalized();

                    Vec2 new_destination = reflect(ball->get_destination(), bounce_normal);

                    float current_ball_speed = ball->get_destination().length();

                    if (new_destination.y > -0.2f)
                        new_destination.y = -0.2f;

                    // Clamp max bounce angle (85 degree)
                    double maxX = std::sin(85.0f * std::numbers::pi / 180.0f);
                    if (new_destination.x > maxX)
                        new_destination.x = maxX;
                    if (new_destination.x < -maxX)
                        new_destination.x = -maxX;

                    ball->set_destination(new_destination.normalized() *
                                          current_ball_speed);

                    Vec2 current_ball_position = ball->get_position();
                    current_ball_position.y = paddel->rety() - ball->radius - 0.1f;
                    ball->set_position(current_ball_position);
                }
            }
            else
            {
                ball->revert_position();
            }
            ball->next_step();
            frontend->draw_background();
            paddel->draw();
            bricksField->draw();
            ball->draw();
            SDL_RenderPresent(renderer);
            if (bricksField->bsize() == 0)
            {
                float elapsed = (SDL_GetTicks() - tstart) / 1000.0f;
                eventManager.getState().gamePreEnd = true;
                while (eventManager.getState().gamePreEnd)
                {
                    frontend->level_cleared(elapsed / std::max(1, score));
                    SDL_RenderPresent(renderer);
                    SDL_Delay(8);
                    frontend->draw_background();
                }
                eventManager.getState().gameStart = false;
            }
        }

        // reset after game over to menu
        SDL_ShowCursor(SDL_ENABLE);
        eventManager.gameStateReset();
        score = 0;
        tstart = SDL_GetTicks();
        bricksField->setmain();
        ball->setmain();
        paddel->setmain();
    }
    cleanup();
    return 0;
}

void Game::cleanup()
{
    eventManager.stopCapture();
    if (music)
    {
        Mix_FreeMusic(music);
        music = nullptr;
    }
    if (renderer)
    {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    if (window)
    {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    Mix_CloseAudio();
    SDL_Quit();
}
