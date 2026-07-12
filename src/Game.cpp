#include "Game.h"
#include "Ball.h"
#include "BlackHole.h"
#include "Bricks.h"
#include "EventManager.h"
#include "FrontendManager.h"
#include "Paddel.h"
#include <SDL_mixer.h>
#include <SDL_rect.h>
#include <SDL_render.h>
#include <SDL_timer.h>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <algorithm>
#include <memory>
#include <numbers>
#include <vector>

Game::Game()
    : playerDb(SNACKS_DIR "/players.txt"), bounds{0, 0, 900, 620}, frontend(nullptr), renderer(nullptr),
      window(nullptr), music(nullptr)
{
}

int Game::run()
{
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << "\n";
        return 1;
    }
    double windowScale = 2.0;
    SDL_Rect displayBounds;
    if (SDL_GetDisplayUsableBounds(0, &displayBounds) == 0)
    {
        double maxScaleW = (displayBounds.w * 0.9) / bounds.w;
        double maxScaleH = (displayBounds.h * 0.9) / bounds.h;
        windowScale = std::clamp(std::min(maxScaleW, maxScaleH), 1.0, 3.0);
    }
    int windowW = (int)(bounds.w * windowScale);
    int windowH = (int)(bounds.h * windowScale);
    window = SDL_CreateWindow("Shalnoi (refactor)", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowW,
                               windowH, SDL_WINDOW_SHOWN);
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
    SDL_RenderSetLogicalSize(renderer, bounds.w, bounds.h);
    eventManager.setLogicalScale((double)windowW / bounds.w, (double)windowH / bounds.h);
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
    blackHole = std::make_unique<BlackHole>(bounds, renderer);
    uint32_t lastInterval = SDL_GetTicks();
    uint32_t animationStart = 0;
    bool animPlaying = false;
    bool ballStuckInHole = false;
    uint32_t ballStuckTime = 0;
    double ballStuckSpeed = 0.0;
    struct PowerUp { Vec2 pos; bool isDuplicate; };
    std::vector<PowerUp> powerUps;
    auto ball2 = std::make_unique<Ball>(bounds, renderer);
    bool ball2Active = false;
    int levelNumber = 0;
    int mx = 0, my = 0;
    int score = 0;
    std::string currentPlayerName;
    long long totalBlocksThisRun = 0;
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

        if (eventManager.getState().gameLeaderboard)
        {
            while (eventManager.getState().gameLeaderboard)
            {
                frontend->draw_background();
                frontend->draw_leaderboard(playerDb.getSorted());
                SDL_RenderPresent(renderer);
                SDL_Delay(8);
            }
            if (eventManager.getState().gameOver)
                break;
            continue;
        }

        if (eventManager.getState().gameNameInput)
        {
            SDL_StartTextInput();
            while (eventManager.getState().gameNameInput)
            {
                frontend->draw_background();
                frontend->draw_name_input(eventManager.getNameInputText());
                SDL_RenderPresent(renderer);
                SDL_Delay(8);
            }
            SDL_StopTextInput();
            if (eventManager.getState().gameOver)
                break;
            if (eventManager.getState().gameMenu)
                continue;
            currentPlayerName = eventManager.getNameInputText();
            if (currentPlayerName.empty())
                currentPlayerName = "Игрок";
            totalBlocksThisRun = 0;
        }

        eventManager.startCaptureEvents();
        eventManager.getState().gameStart = false;
        eventManager.getState().gamePaused = false;
        eventManager.getState().gameOver = false;
        bricksField->load_level(levelNumber);
        ball->setmain();
        paddel->setmain();
        blackHole->resetTimer();
        ballStuckInHole = false;
        ball2Active = false;
        ball2->setmain();
        powerUps.clear();
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
            SDL_GetMouseState(&mx, &my);
            float logicalMx, logicalMy;
            SDL_RenderWindowToLogical(renderer, mx, my, &logicalMx, &logicalMy);
            mx = (int)logicalMx;
            if (mx >= 50 && mx <= 830)
            {
                paddel->setpos(mx - 50);
            }

            frontend->draw_background();
            if (ball->get_position().y >= (float)(bounds.h - 10))
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
            else if (!ball->out_of_bounds() && !ballStuckInHole)
            {
                for (int i = 0; i < bricksField->remaining(); ++i)
                {
                    auto &current_block = bricksField->retBlock(i);
                    if (auto closest_point = ball->touches(current_block); closest_point.has_value())
                    {
                        bool isStone = (bricksField->retvar(i) < 0);
                        if (!ball->isFireBall() || isStone)
                        {
                            Vec2 normal = ball->get_position() - closest_point.value();
                            double dist = normal.length();
                            if (dist > 0.0)
                            {
                                Vec2 n = normal / dist;
                                Vec2 vel = ball->get_destination();
                                ball->set_destination(vel - n * (2.0 * vel.dot(n)));
                                Vec2 np = ball->get_position() + n * (ball->radius - dist + 0.5);
                                np.x = std::clamp(np.x, (double)ball->radius, (double)(bounds.w - ball->radius));
                                np.y = std::clamp(np.y, (double)ball->radius, (double)(bounds.h - ball->radius));
                                ball->set_position(np);
                            }
                            else
                            {
                                SDL_Rect r = current_block.r;
                                Vec2 pos = ball->get_position();
                                double fL = pos.x - r.x, fR = r.x + r.w - pos.x;
                                double fT = pos.y - r.y, fB = r.y + r.h - pos.y;
                                Vec2 pd{-1.0, 0.0};
                                double mp = fL;
                                if (fR < mp) { mp = fR; pd = {1.0, 0.0}; }
                                if (fT < mp) { mp = fT; pd = {0.0, -1.0}; }
                                if (fB < mp) { mp = fB; pd = {0.0, 1.0}; }
                                Vec2 vel = ball->get_destination();
                                ball->set_destination(vel - pd * (2.0 * vel.dot(pd)));
                                ball->set_position(pos + pd * (ball->radius + mp + 0.5));
                            }
                        }
                        if (!isStone)
                        {
                            bricksField->minus(i);
                            if (bricksField->retvar(i) == 0)
                            {
                                Vec2 bc{current_block.r.x + current_block.r.w / 2.0,
                                        current_block.r.y + current_block.r.h / 2.0};
                                bricksField->del(i);
                                score++;
                                totalBlocksThisRun++;
                                i--;
                                if (std::rand() % 100 < 7)
                                    powerUps.push_back({bc, std::rand() % 2 == 0});
                            }
                        }
                        if (!ball->isFireBall() || isStone)
                            break;
                    }
                }

                if (auto closest_point = ball->touches(*paddel); closest_point.has_value())
                {
                    Vec2 closest_point_vec = closest_point.value();
                    double paddel_width = (double)paddel->retw();
                    double paddel_center_x = paddel->retx() + paddel_width / 2.0;
                    double offset = (closest_point_vec.x - paddel_center_x) / (paddel_width / 2.0);
                    offset = std::max(-1.0, std::min(1.0, offset));

                    constexpr double maxAngle = 75.0 * std::numbers::pi / 180.0;
                    double angle = offset * maxAngle;
                    const double canonicalSpeed = (double)bounds.w / bounds.h;
                    ball->set_destination({std::sin(angle) * canonicalSpeed,
                                          -std::cos(angle) * canonicalSpeed});

                    Vec2 current_ball_position = ball->get_position();
                    current_ball_position.y = paddel->rety() - ball->radius - 0.1f;
                    ball->set_position(current_ball_position);
                }
            }
            else
            {
                ball->revert_position();
            }

            if (!ballStuckInHole &&
                bricksField->destructibleCount() <= (int)(bricksField->getDestructibleStartingSize() * 2.0f / 3.0f) &&
                blackHole->isActive())
            {
                SDL_Rect bhRect = blackHole->getRect();
                Vec2 closest = closestPoint(bhRect, ball->get_position());
                if ((ball->get_position() - closest).length() < ball->radius)
                {
                    ballStuckInHole = true;
                    ballStuckTime = SDL_GetTicks();
                    ballStuckSpeed = ball->get_destination().length();
                    ball->set_destination({0.0, 0.0});
                }
            }

            if (ballStuckInHole)
            {
                SDL_Rect bhRect = blackHole->getRect();
                ball->set_position({bhRect.x + bhRect.w / 2.0, bhRect.y + bhRect.h / 2.0});
                if (SDL_GetTicks() - ballStuckTime >= 1000)
                {
                    ballStuckInHole = false;
                    double angle = (std::rand() % 360) * std::numbers::pi / 180.0;
                    Vec2 newDir{std::cos(angle), std::sin(angle)};
                    if (std::abs(newDir.y) < 0.2)
                    {
                        newDir.y = newDir.y >= 0 ? 0.2 : -0.2;
                        newDir = newDir.normalized();
                    }
                    ball->set_position(ball->get_position() + newDir * (50.0 + ball->radius));
                    ball->set_destination(newDir * ballStuckSpeed);
                }
            }

            ball->updateFireBall();
            if (!ballStuckInHole)
                ball->next_step();

            if (ball2Active)
            {
                if (ball2->get_position().y >= (float)(bounds.h - 10))
                {
                    ball2Active = false;
                }
                else
                {
                    if (!ball2->out_of_bounds())
                    {
                        for (int i = 0; i < bricksField->remaining(); ++i)
                        {
                            auto &cb = bricksField->retBlock(i);
                            if (auto cp = ball2->touches(cb); cp.has_value())
                            {
                                bool isStone = (bricksField->retvar(i) < 0);
                                if (!ball2->isFireBall() || isStone)
                                {
                                    Vec2 normal = ball2->get_position() - cp.value();
                                    double dist = normal.length();
                                    if (dist > 0.0)
                                    {
                                        Vec2 n = normal / dist;
                                        Vec2 vel = ball2->get_destination();
                                        ball2->set_destination(vel - n * (2.0 * vel.dot(n)));
                                        Vec2 np = ball2->get_position() + n * (ball2->radius - dist + 0.5);
                                        np.x = std::clamp(np.x, (double)ball2->radius, (double)(bounds.w - ball2->radius));
                                        np.y = std::clamp(np.y, (double)ball2->radius, (double)(bounds.h - ball2->radius));
                                        ball2->set_position(np);
                                    }
                                    else
                                    {
                                        SDL_Rect r2 = cb.r;
                                        Vec2 pos = ball2->get_position();
                                        double fL = pos.x-r2.x, fR = r2.x+r2.w-pos.x, fT = pos.y-r2.y, fB = r2.y+r2.h-pos.y;
                                        Vec2 pd{-1.0, 0.0}; double mp = fL;
                                        if (fR<mp){mp=fR;pd={1.0,0.0};} if(fT<mp){mp=fT;pd={0.0,-1.0};} if(fB<mp){mp=fB;pd={0.0,1.0};}
                                        Vec2 vel = ball2->get_destination();
                                        ball2->set_destination(vel - pd * (2.0 * vel.dot(pd)));
                                        ball2->set_position(pos + pd * (ball2->radius + mp + 0.5));
                                    }
                                }
                                if (!isStone)
                                {
                                    bricksField->minus(i);
                                    if (bricksField->retvar(i) == 0)
                                    {
                                        Vec2 bc{cb.r.x + cb.r.w / 2.0, cb.r.y + cb.r.h / 2.0};
                                        bricksField->del(i);
                                        score++;
                                        totalBlocksThisRun++;
                                        i--;
                                        if (std::rand() % 100 < 7)
                                            powerUps.push_back({bc, std::rand() % 2 == 0});
                                    }
                                }
                                if (!ball2->isFireBall() || isStone)
                                    break;
                            }
                        }
                        if (auto cp = ball2->touches(*paddel); cp.has_value())
                        {
                            Vec2 cpv = cp.value();
                            double pw2 = (double)paddel->retw();
                            double pcx2 = paddel->retx() + pw2 / 2.0;
                            double off2 = std::clamp((cpv.x - pcx2) / (pw2 / 2.0), -1.0, 1.0);
                            const double cs2 = (double)bounds.w / bounds.h;
                            double ang2 = off2 * (75.0 * std::numbers::pi / 180.0);
                            ball2->set_destination({std::sin(ang2) * cs2, -std::cos(ang2) * cs2});
                            Vec2 p2 = ball2->get_position();
                            p2.y = paddel->rety() - ball2->radius - 0.1;
                            ball2->set_position(p2);
                        }
                    }
                    else
                    {
                        ball2->revert_position();
                    }
                    ball2->updateFireBall();
                    ball2->next_step();
                }
            }

            frontend->draw_background();
            paddel->draw();
            bricksField->draw();

            for (int i = (int)powerUps.size() - 1; i >= 0; i--)
            {
                powerUps[i].pos.y += 2.0;
                bool hit = powerUps[i].pos.y + 7 >= paddel->rety() &&
                           powerUps[i].pos.y - 7 <= paddel->rety() + 10 &&
                           powerUps[i].pos.x + 15 >= paddel->retx() &&
                           powerUps[i].pos.x - 15 <= paddel->retx() + paddel->retw();
                if (hit)
                {
                    if (powerUps[i].isDuplicate)
                    {
                        if (!ball2Active)
                        {
                            ball2Active = true;
                            ball2->set_position(ball->get_position());
                            Vec2 d = ball->get_destination();
                            ball2->set_destination({-d.x, d.y});
                            ball2->radius = 10;
                            if (ball->isFireBall())
                                ball2->activateFireBall(4000);
                        }
                    }
                    else
                    {
                        ball->activateFireBall(4000);
                        if (ball2Active)
                            ball2->activateFireBall(4000);
                    }
                    powerUps.erase(powerUps.begin() + i);
                }
                else if (powerUps[i].pos.y > bounds.h + 20)
                {
                    powerUps.erase(powerUps.begin() + i);
                }
                else
                {
                    SDL_Rect pr = {(int)powerUps[i].pos.x - 15, (int)powerUps[i].pos.y - 7, 30, 14};
                    if (powerUps[i].isDuplicate)
                        SDL_SetRenderDrawColor(renderer, 0, 150, 255, 255);
                    else
                        SDL_SetRenderDrawColor(renderer, 255, 120, 0, 255);
                    SDL_RenderFillRect(renderer, &pr);
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                    SDL_RenderDrawRect(renderer, &pr);
                }
            }

            if (ball2Active)
                ball2->draw();
            ball->draw();
            if (bricksField->destructibleCount() <= (int)(bricksField->getDestructibleStartingSize() * 2.0f / 3.0f))
            {
                blackHole->draw();
            }
            SDL_RenderPresent(renderer);
            if (bricksField->destructibleCount() == 0)
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
                levelNumber++;
                bricksField->load_level(levelNumber);
                ball->setmain();
                paddel->setmain();
                blackHole->resetTimer();
                ballStuckInHole = false;
                ball2Active = false;
                ball2->setmain();
                powerUps.clear();
                score = 0;
                tstart = SDL_GetTicks();
                // Show welcome screen before next level starts
                eventManager.getState().gameWelcome = true;
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
            }
        }

        // reset after game over to menu
        SDL_ShowCursor(SDL_ENABLE);
        if (!currentPlayerName.empty())
        {
            playerDb.addBlocks(currentPlayerName, totalBlocksThisRun);
            currentPlayerName.clear();
        }
        totalBlocksThisRun = 0;
        if (eventManager.getState().appQuit)
            break;
        eventManager.gameStateReset();
        score = 0;
        levelNumber = 0;
        tstart = SDL_GetTicks();
        bricksField->setmain();
        ball->setmain();
        ball2->setmain();
        ball2Active = false;
        powerUps.clear();
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
