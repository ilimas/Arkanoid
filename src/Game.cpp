#include "Game.h"
#include "Ball.h"
#include "BlackHole.h"
#include "Bricks.h"
#include "Clock.h"
#include "EventManager.h"
#include "FrontendManager.h"
#include "Paddel.h"
#include "ResolutionPresets.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <memory>
#include <numbers>
#include <utility>
#include <vector>

namespace
{
// GLFW doesn't have an SDL_WINDOWPOS_CENTERED-style creation flag - center
// manually against the primary monitor's video mode after creating/resizing.
void centerWindow(GLFWwindow *window, int w, int h)
{
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    if (!monitor)
        return;
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    if (!mode)
        return;
    glfwSetWindowPos(window, (mode->width - w) / 2, (mode->height - h) / 2);
}
} // namespace

Game::Game()
    : playerDb(SNACKS_DIR "/players.txt"), bounds{0, 0, 1440, 992}, frontend(nullptr), window(nullptr)
{
}

int Game::run()
{
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    if (!glfwInit())
    {
        std::cerr << "glfwInit failed\n";
        return 1;
    }

    double windowScale = 2.0;
    if (GLFWmonitor *monitor = glfwGetPrimaryMonitor())
    {
        int workX, workY, workW, workH;
        glfwGetMonitorWorkarea(monitor, &workX, &workY, &workW, &workH);
        double maxScaleW = (workW * 0.9) / bounds.w;
        double maxScaleH = (workH * 0.9) / bounds.h;
        windowScale = std::clamp(std::min(maxScaleW, maxScaleH), 1.0, 3.0);
    }
    int windowW = (int)(bounds.w * windowScale);
    int windowH = (int)(bounds.h * windowScale);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_DEPTH_BITS, 0);
    glfwWindowHint(GLFW_STENCIL_BITS, 0);

    window = glfwCreateWindow(windowW, windowH, "Shalnoi (refactor)", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "glfwCreateWindow failed\n";
        glfwTerminate();
        return 1;
    }
    centerWindow(window, windowW, windowH);
    glfwShowWindow(window);

    // Every draw in the game goes through GLRenderer (raw OpenGL, GL 3.3 core)
    // instead of a windowing toolkit's own 2D renderer - see GLRenderer.h.
    try
    {
        gl = std::make_unique<GLRenderer>(window);
    }
    catch (const std::exception &e)
    {
        std::cerr << "GLRenderer init failed: " << e.what() << "\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }
    glfwSwapInterval(1); // vsync: avoids tearing/uneven pacing (see limitFrameRate below)
    lastFrameTime_ = std::chrono::steady_clock::now();
    eventManager.attachWindow(window);
    eventManager.setViewport(gl->viewportX(), gl->viewportY(), gl->viewportW(), gl->viewportH());

    frontend = std::make_unique<FrontendManager>(*gl);
    paddel = std::make_unique<Paddel>(*gl);
    bricksField = std::make_unique<BlockField>(*gl);
    ball = std::make_unique<Ball>(bounds, *gl);
    blackHole = std::make_unique<BlackHole>(bounds, *gl);
    bool ballStuckInHole = false;
    uint32_t ballStuckTime = 0;
    double ballStuckSpeed = 0.0;
    struct PowerUp { Vec2 pos; bool isDuplicate; };
    std::vector<PowerUp> powerUps;
    ball2 = std::make_unique<Ball>(bounds, *gl);
    bool ball2Active = false;
    int levelNumber = 0;
    int mx = 0;
    int score = 0;
    std::string currentPlayerName;
    long long totalBlocksThisRun = 0;
    int currentResIndex = -1; // which preset (if any) matches the active window size, for the Settings screen
    uint32_t tstart = nowMs();
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    while (!eventManager.getState().gameOver)
    {
        while (eventManager.getState().gameMenu)
        {
            eventManager.pollEvents();
            gl->beginFrame();
            frontend->draw_menu(eventManager.getState().menuSelectedItem);
            gl->present(window);
            limitFrameRate();
        }
        if (eventManager.getState().gameOver)
            break;

        if (eventManager.getState().gameLeaderboard)
        {
            while (eventManager.getState().gameLeaderboard)
            {
                eventManager.pollEvents();
                gl->beginFrame();
                frontend->draw_background();
                frontend->draw_leaderboard(playerDb.getSorted());
                gl->present(window);
                limitFrameRate();
            }
            if (eventManager.getState().gameOver)
                break;
            continue;
        }

        if (eventManager.getState().gameSettings)
        {
            while (eventManager.getState().gameSettings)
            {
                eventManager.pollEvents();
                int idx = eventManager.getState().pendingResolutionIndex;
                if (idx >= 0)
                {
                    eventManager.getState().pendingResolutionIndex = -1;
                    if (idx < kResolutionPresetCount)
                    {
                        applyResolution(kResolutionPresets[idx].w, kResolutionPresets[idx].h);
                        currentResIndex = idx;
                    }
                }
                gl->beginFrame();
                frontend->draw_background();
                frontend->draw_settings(eventManager.getState().settingsSelectedItem, currentResIndex);
                gl->present(window);
                limitFrameRate();
            }
            if (eventManager.getState().gameOver)
                break;
            continue;
        }

        if (eventManager.getState().gameNameInput)
        {
            while (eventManager.getState().gameNameInput)
            {
                eventManager.pollEvents();
                gl->beginFrame();
                frontend->draw_background();
                frontend->draw_name_input(eventManager.getNameInputText());
                gl->present(window);
                limitFrameRate();
            }
            if (eventManager.getState().gameOver)
                break;
            if (eventManager.getState().gameMenu)
                continue;
            currentPlayerName = eventManager.getNameInputText();
            if (currentPlayerName.empty())
                currentPlayerName = "Игрок";
            totalBlocksThisRun = 0;
        }

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
        tstart = nowMs();

        while (eventManager.getState().gameWelcome)
        {
            eventManager.pollEvents();
            gl->beginFrame();
            frontend->draw_background();
            paddel->draw();
            bricksField->draw();
            ball->draw();
            frontend->draw_welcome_text();
            gl->present(window);
            limitFrameRate();
        }

        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
        uint32_t lastPhysicsTicks = nowMs();
        bool justResumed = false;
        while (eventManager.getState().gameStart)
        {
            bool wasPaused = eventManager.getState().gamePaused;
            while (eventManager.getState().gamePaused)
            {
                eventManager.pollEvents();
                gl->beginFrame();
                frontend->draw_background();
                frontend->draw_pause();
                gl->present(window);
                limitFrameRate();
            }
            if (wasPaused)
                justResumed = true;
            if (eventManager.getState().gameOver == true)
                break;

            eventManager.pollEvents();
            gl->beginFrame();

            // Frame-rate independent timestep: everything below moves at a
            // px/sec rate scaled by dt, not a fixed amount per rendered frame,
            // so gameplay speed stays the same regardless of the actual frame
            // rate. dt is 0 for the first frame after a pause so the paused
            // duration itself never turns into a single oversized physics step,
            // and it's clamped so any other hitch can't tunnel the ball through
            // the paddle/bricks either.
            uint32_t nowTicks = nowMs();
            double dt = justResumed ? 0.0 : std::min((nowTicks - lastPhysicsTicks) / 1000.0, 0.05);
            lastPhysicsTicks = nowTicks;
            justResumed = false;

            double rawMx, rawMy;
            glfwGetCursorPos(window, &rawMx, &rawMy);
            Point logical = gl->windowToLogical((int)rawMx, (int)rawMy);
            mx = logical.x;
            if (mx >= 80 && mx <= 1328)
            {
                paddel->setpos(mx - 80);
            }

            frontend->draw_background();
            if (ball->get_position().y >= (float)(bounds.h - 16))
            {
                if (ball2Active)
                {
                    // The main ball dropped but the duplicate is still in play:
                    // promote it to be the main ball instead of ending the game.
                    std::swap(ball, ball2);
                    ball2Active = false;
                    ballStuckInHole = false;
                }
                else
                {
                    eventManager.getState().gamePreEnd = true;
                    while (eventManager.getState().gamePreEnd)
                    {
                        eventManager.pollEvents();
                        gl->beginFrame();
                        frontend->draw_background();
                        frontend->draw_end();
                        gl->present(window);
                        limitFrameRate();
                    };
                    break;
                }
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
                                Rect r = current_block.r;
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
                Rect bhRect = blackHole->getRect();
                Vec2 closest = closestPoint(bhRect, ball->get_position());
                if ((ball->get_position() - closest).length() < ball->radius)
                {
                    ballStuckInHole = true;
                    ballStuckTime = nowMs();
                    ballStuckSpeed = ball->get_destination().length();
                    ball->set_destination({0.0, 0.0});
                }
            }

            if (ballStuckInHole)
            {
                Rect bhRect = blackHole->getRect();
                ball->set_position({bhRect.x + bhRect.w / 2.0, bhRect.y + bhRect.h / 2.0});
                if (nowMs() - ballStuckTime >= 1000)
                {
                    ballStuckInHole = false;
                    double angle = (std::rand() % 360) * std::numbers::pi / 180.0;
                    Vec2 newDir{std::cos(angle), std::sin(angle)};
                    if (std::abs(newDir.y) < 0.2)
                    {
                        newDir.y = newDir.y >= 0 ? 0.2 : -0.2;
                        newDir = newDir.normalized();
                    }
                    ball->set_position(ball->get_position() + newDir * (80.0 + ball->radius));
                    ball->set_destination(newDir * ballStuckSpeed);
                }
            }

            ball->updateFireBall();
            if (!ballStuckInHole)
                ball->next_step(dt);

            if (ball2Active)
            {
                if (ball2->get_position().y >= (float)(bounds.h - 16))
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
                                        Rect r2 = cb.r;
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
                    ball2->next_step(dt);
                }
            }

            frontend->draw_background();
            paddel->draw();
            bricksField->draw();

            constexpr double powerUpFallSpeed = 192.0; // px/sec
            for (int i = (int)powerUps.size() - 1; i >= 0; i--)
            {
                powerUps[i].pos.y += powerUpFallSpeed * dt;
                bool hit = powerUps[i].pos.y + 11 >= paddel->rety() &&
                           powerUps[i].pos.y - 11 <= paddel->rety() + 16 &&
                           powerUps[i].pos.x + 24 >= paddel->retx() &&
                           powerUps[i].pos.x - 24 <= paddel->retx() + paddel->retw();
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
                            ball2->radius = 16;
                            ball2->setSpeedElapsed(ball->getSpeedElapsed());
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
                    Rect pr = {(int)powerUps[i].pos.x - 24, (int)powerUps[i].pos.y - 11, 48, 22};
                    Color fill = powerUps[i].isDuplicate ? Color{0, 150, 255, 255} : Color{255, 120, 0, 255};
                    gl->drawColor(pr, fill);
                    gl->drawColorOutline(pr, Color{255, 255, 255, 255});
                }
            }

            if (ball2Active)
                ball2->draw();
            ball->draw();
            if (bricksField->destructibleCount() <= (int)(bricksField->getDestructibleStartingSize() * 2.0f / 3.0f))
            {
                blackHole->draw();
            }
            frontend->draw_hud(score, levelNumber);
            gl->present(window);
            limitFrameRate();
            if (bricksField->destructibleCount() == 0)
            {
                float elapsed = (nowMs() - tstart) / 1000.0f;
                eventManager.getState().gamePreEnd = true;
                while (eventManager.getState().gamePreEnd)
                {
                    eventManager.pollEvents();
                    gl->beginFrame();
                    frontend->draw_background();
                    frontend->level_cleared(elapsed / std::max(1, score));
                    gl->present(window);
                    limitFrameRate();
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
                tstart = nowMs();
                // Show welcome screen before next level starts
                eventManager.getState().gameWelcome = true;
                while (eventManager.getState().gameWelcome)
                {
                    eventManager.pollEvents();
                    gl->beginFrame();
                    frontend->draw_background();
                    paddel->draw();
                    bricksField->draw();
                    ball->draw();
                    frontend->draw_welcome_text();
                    gl->present(window);
                    limitFrameRate();
                }
                justResumed = true;
            }
        }

        // reset after game over to menu
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
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
        tstart = nowMs();
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

void Game::applyResolution(int w, int h)
{
    // The logical canvas (UILayout::ScreenW x ScreenH) never changes - only
    // the window's actual pixel size does. GLRenderer letterboxes the fixed
    // canvas to fit the new window; EventManager's viewport must be updated
    // to match so mouse hit-testing still lines up.
    glfwSetWindowSize(window, w, h);
    centerWindow(window, w, h);
    gl->resize(w, h);
    eventManager.setViewport(gl->viewportX(), gl->viewportY(), gl->viewportW(), gl->viewportH());
}

void Game::limitFrameRate()
{
    constexpr double targetFrameSeconds = 1.0 / 120.0;
    auto now = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double>(now - lastFrameTime_).count();
    if (elapsed < targetFrameSeconds)
        sleepMs((uint32_t)((targetFrameSeconds - elapsed) * 1000.0));
    lastFrameTime_ = std::chrono::steady_clock::now();
}

void Game::cleanup()
{
    // Every GL-texture-owning object must be destroyed before the GL context
    // (gl) itself, and gl must be destroyed before the window it was created
    // on - order here matters, unlike a plain member-wise destructor.
    blackHole.reset();
    ball2.reset();
    ball.reset();
    bricksField.reset();
    paddel.reset();
    frontend.reset();
    gl.reset();
    if (window)
    {
        glfwDestroyWindow(window);
        window = nullptr;
    }
    glfwTerminate();
}
