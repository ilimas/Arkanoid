#include "EventManager.h"
#include <SDL_events.h>
#include <atomic>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <optional>
#include <thread>

GameState::GameState() : gameStart(false),
                         gameWelcome(false),
                         gamePaused(false),
                         gameOver(false),
                         gamePreEnd(false),
                         gameMenu(true),
                         menuSelectedItem(0)
{
}

GameState &EventManager::getState()
{
    return currentState;
}

void EventManager::gameStateReset()
{
    currentState.gameStart = false;
    currentState.gamePaused = false;
    currentState.gameOver = false;
    currentState.gameMenu = true;
}

void EventManager::waitForEvent()
{
    std::unique_lock<std::mutex> lock(mutex_);
    condition_.wait(lock, [&]
                    { return !queue_.empty() || getState().gameOver; });
}

std::optional<SDL_Event> EventManager::popEvent()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (queue_.empty())
        return std::nullopt;
    SDL_Event e = queue_.front();
    queue_.pop_front();
    return e;
}

bool EventManager::tryPopEvent(SDL_Event &out)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (queue_.empty())
        return false;
    out = queue_.front();
    queue_.pop_front();
    return true;
}

void EventManager::clearQueue()
{
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.clear();
}

void EventManager::startCaptureEvents()
{
    pollThread = std::jthread(&EventManager::acceptEventsWorker, this);
    eventThread = std::jthread(&EventManager::processEventsWorker, this);
}

void EventManager::stopCapture()
{
    clearQueue();
    eventThread.request_stop();
    pollThread.request_stop();
    eventThread.join();
    pollThread.join();
}

void EventManager::acceptEventsWorker(std::stop_token stopToken)
{
    SDL_Event ev;
    while (!stopToken.stop_requested())
    {
        while (SDL_PollEvent(&ev))
        {
            if (stopToken.stop_requested() || ev.type == SDL_QUIT)
            {
                condition_.notify_one();
                return;
            }
            {
                std::lock_guard<std::mutex> lock(mutex_);
                queue_.push_back(ev);
            }
            condition_.notify_one();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void EventManager::processEventsWorker(std::stop_token stopToken)
{
    while (!stopToken.stop_requested())
    {
        SDL_Event event;
        int mouseX, mouseY;
        if (tryPopEvent(event))
        {
            if (currentState.gameMenu)
            {
                SDL_GetMouseState(&mouseX, &mouseY);
                if (mouseX >= 450 && mouseX <= 600 && mouseY <= 130 && mouseY >= 100)
                {
                    currentState.menuSelectedItem.store(0);
                    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)
                    {
                        currentState.gameWelcome = true;
                        currentState.gameMenu = false;
                    }
                }
                else if (mouseX >= 450 && mouseX <= 600 && mouseY <= 170 && mouseY >= 140)
                {
                    currentState.menuSelectedItem.store(1);
                    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)
                    {
                        currentState.gameOver = true;
                        currentState.gameMenu = false;
                    }
                }
                else if (event.type == SDL_QUIT)
                {
                    currentState.menuSelectedItem.store(0);
                    currentState.gameOver = true;
                    currentState.gameMenu = false;
                }
            }
            else if (currentState.gameWelcome)
            {
                if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)
                {
                    currentState.gameStart = true;
                    currentState.gameWelcome = false;
                }
                else if (event.type == SDL_QUIT)
                {
                    currentState.gameOver = true;
                    currentState.gameWelcome = false;
                }
            }
            else if (currentState.gamePreEnd)
            {
                if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RETURN)
                {
                    currentState.gamePreEnd = false;
                }
            }
            else if (currentState.gamePaused)
            {
                if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
                {
                    currentState.gamePaused = false;
                }
                else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RETURN)
                {
                    currentState.gameStart = false;
                    currentState.gamePaused = false;
                    currentState.gameOver = true;
                }
            }
            else if (currentState.gameStart)
            {
                if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
                {
                    currentState.gamePaused = true;
                }
            }
            else if (currentState.gameOver)
            {
                break;
            }
        }
    }
}
