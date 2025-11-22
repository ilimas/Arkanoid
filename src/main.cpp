#include "bblock.h"
#include "blocks.h"
#include "image.h"
#include "shar.h"
#include "utils.h"
#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_rect.h>
#include <SDL_ttf.h>
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <math.h>
#include <mutex>
#include <numbers>
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <time.h>

// TODO: Move logic to multiple classes
// Leave only "run" function in main

#define endlessLoop while (true)

extern TTF_Font *fnt;
std::deque<SDL_Event> eventQueue;
std::mutex queueMutex;
std::atomic<bool> gameStartFlag = false, gamePaused = false, gameOver = false;
std::condition_variable eventCondition;

void eventThread()
{
    SDL_Event event;
    while (true)
    {
        if (gameOver)
            return;
        while (SDL_PollEvent(&event))
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            eventQueue.push_back(event);
            eventCondition.notify_one();
        }
        SDL_Delay(1);
    }
}

void pauseThread()
{
    SDL_Event event;
    while (true)
    {
        if (gameOver)
            return;
        if (gameStartFlag)
        {
            bool emptyQueue;
            {
                std::unique_lock<std::mutex> lock(queueMutex);
                emptyQueue = eventQueue.empty();
            }
            if (!emptyQueue)
            {
                event = eventQueue.front();
                eventQueue.pop_front();
                if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
                {
                    gamePaused = !gamePaused;
                }
                else if (gamePaused && event.type == SDL_KEYDOWN &&
                         event.key.keysym.sym == SDLK_RETURN)
                {
                    gameStartFlag = false;
                    gamePaused = false;
                    gameOver = true;
                    return;
                }
            }
        }
    }
}

int main(int argc, char *args[])
{
    int tstart = SDL_GetTicks();
    int n = 0, score = 0;
    Mix_Music *music = NULL;
    constexpr uint32_t width = 640;
    constexpr uint32_t height = 480;
    SDL_Rect bounds{0, 0, width, height};
    SDL_Event event;
    int mx, my;
    double ti = 1;
    srand(time(0));
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }
    if (TTF_Init() != 0)
        printf("Error ttf");
    SDL_Window *win = SDL_CreateWindow("Shalnoi", 20, 100, bounds.w, bounds.h,
                                       SDL_WINDOW_SHOWN);
    if (!win)
    {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        return 1;
    }
    SDL_Renderer *ren = SDL_CreateRenderer(
        win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!ren)
    {
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        return 1;
    }
    BlockField block_field;
    Ball ball(bounds);
    Paddel paddel(ren);
    FrontendManager painter(ren);
    if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) == -1)
        return 2;
    music = Mix_LoadMUS(SNACKS_DIR "/music/shrek.wav");
    if (!music)
        return 1;
    SDL_RenderClear(ren);
    if (Mix_PlayMusic(music, -1) == -1)
        return 1;
    endlessLoop
    {
        std::thread eventProcessing(eventThread);
        eventProcessing.detach();
        endlessLoop
        {
            painter.draw_menu(n);
            SDL_RenderPresent(ren);
            SDL_GetMouseState(&mx, &my);
            if (mx >= 450 && mx <= 600 && my <= 130 && my >= 100)
                n = 0;
            else if (mx >= 450 && mx <= 600 && my <= 170 && my >= 140)
                n = 1;
            {
                std::unique_lock<std::mutex> lock(queueMutex);
                eventCondition.wait(lock, []()
                                    { return !eventQueue.empty(); });
                while (!eventQueue.empty())
                {
                    event = eventQueue.front();
                    eventQueue.pop_front();
                    if (event.type == SDL_MOUSEBUTTONDOWN &&
                        event.button.button == SDL_BUTTON_LEFT && mx <= 600 &&
                        mx >= 450 && my >= 100 && my <= 170)
                    {
                        if (n == 0)
                        {
                            block_field.load_level();
                            goto jmp_game_start;
                        }
                        else if (n == 1)
                        {
                            Mix_FreeMusic(music);
                            SDL_DestroyRenderer(ren);
                            SDL_DestroyWindow(win);
                            Mix_CloseAudio();
                            TTF_CloseFont(fnt);
                            TTF_Quit();
                            SDL_Quit();
                            return 3;
                        }
                    }
                }
            }
        }
    jmp_game_start:
        SDL_RenderPresent(ren);
        SDL_GetMouseState(&mx, &my);
        while (!gameOver)
        {
            if (!gameStartFlag)
            {
                painter.draw_background();
                paddel.Draw_Block();
                block_field.Draw_Blocks(ren);
                ball.Draw(ren);
                painter.draw_begin();
                SDL_RenderPresent(ren);
                while (!gameStartFlag)
                {
                    std::unique_lock<std::mutex> lock(queueMutex);
                    eventCondition.wait(lock, []()
                                        { return !eventQueue.empty(); });
                    while (!eventQueue.empty())
                    {
                        event = eventQueue.front();
                        eventQueue.pop_front();
                        if (event.type == SDL_MOUSEBUTTONDOWN &&
                            event.button.button == SDL_BUTTON_LEFT)
                        {
                            gameStartFlag = true;
                            break;
                        }
                    }
                }
                std::thread pauseProcessing(pauseThread);
                pauseProcessing.detach();
            }
            if (gamePaused)
            {
                painter.draw_pause();
                SDL_RenderPresent(ren);
                while (gamePaused)
                    ;
            }
            SDL_GetMouseState(&mx, NULL);
            if (mx >= 0 && mx <= 540)
            {
                paddel.setpos(mx);
            }
            if (gameStartFlag)
            {
                SDL_ShowCursor(SDL_DISABLE);
                painter.draw_background();
                if (ball.get_position().y >= 470)
                {
                    painter.draw_end();
                    SDL_RenderPresent(ren);
                    while (1)
                    {
                        if (SDL_PollEvent(&event) && event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RETURN)
                        {
                            gameOver = true;
                            break;
                        }
                    }
                }
                if (!ball.out_of_bounds())
                {
                    auto &blocks_vector = block_field.getBlocksVector();
                    for (int i = 0; i < block_field.bsize(); ++i)
                    {
                        auto &current_block = block_field.retBlock(i);
                        if (auto closest_point = ball.touches(current_block); closest_point.has_value())
                        {
                            HitSide side = current_block.detectHitSide(ball.get_position(), ball.radius);
                            ball.set_destination(current_block.reflectBall(ball));
                            auto normal = ball.get_position() - closest_point.value();
                            auto distance = normal.length();
                            auto penetration = ball.radius - distance;
                            ball.set_position(ball.get_position() + normal * (penetration + 0.1f));
                            block_field.minus(i);
                            if (block_field.retvar(i) == 0)
                            {
                                block_field.del(i);
                                score++;
                            }
                        }
                    }
                    if (auto closest_point = ball.touches(paddel); closest_point.has_value())
                    {
                        Vec2 closest_point_vec = closest_point.value();
                        int paddel_width = 100;
                        double paddel_center_x = paddel.retx() + paddel_width / 2.f;
                        double offset =
                            (closest_point_vec.x - paddel_center_x) / (paddel_width / 2.f);
                        if (offset < -1.f)
                            offset = -1.f;
                        if (offset > 1.f)
                            offset = 1.f;

                        Vec2 bounce_normal{offset, -1.f};
                        bounce_normal = bounce_normal.normalized();

                        Vec2 new_destination = reflect(ball.get_destination(), bounce_normal);

                        float current_ball_speed = ball.get_destination().length();

                        if (new_destination.y > -0.2f)
                            new_destination.y = -0.2f;

                        // Clamp max bounce angle (75 degree)
                        double maxX = std::sin(75.0f * std::numbers::pi / 180.0f);

                        if (new_destination.x > maxX)
                            new_destination.x = maxX;
                        if (new_destination.x < -maxX)
                            new_destination.x = -maxX;

                        ball.set_destination(new_destination.normalized() *
                                             current_ball_speed);

                        Vec2 current_ball_position = ball.get_position();
                        current_ball_position.y = paddel.rety() - ball.radius - 0.1f;
                        ball.set_position(current_ball_position);
                    }
                }
                else
                {
                    ball.revert_position();
                }
                ball.next_step();
                paddel.Draw_Block();
                block_field.Draw_Blocks(ren);
                ball.Draw(ren);
                SDL_RenderPresent(ren);
            }
            if (block_field.bsize() == 0)
            {
                ti = SDL_GetTicks() - tstart;
                ti /= 1000;
                painter.level_cleared(ti / score);
                SDL_RenderPresent(ren);
                while (1)
                {
                    if (SDL_PollEvent(&event) && event.type == SDL_KEYDOWN &&
                        event.key.keysym.sym == SDLK_RETURN)
                    {
                        gameOver = true;
                        break;
                    }
                }
            }
        }
        SDL_ShowCursor(SDL_ENABLE);
        gameStartFlag = false;
        gameOver = false;
        gamePaused = false;
        score = 0;
        tstart = SDL_GetTicks();
        block_field.setmain();
        ball.setmain();
        paddel.setmain();
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            eventQueue.clear();
        }
    }
    Mix_FreeMusic(music);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    Mix_CloseAudio();
    TTF_Quit();
    SDL_Quit();
    return 0;
}
