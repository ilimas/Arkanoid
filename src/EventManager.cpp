#include "EventManager.h"
#include "UILayout.h"
#include <SDL_events.h>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>
#include <cstring>

GameState::GameState()
    : gameStart(false), gameWelcome(false), gamePaused(false), gameOver(false), gamePreEnd(false),
      gameMenu(true), gameNameInput(false), gameLeaderboard(false), appQuit(false), menuSelectedItem(0)
{
}

GameState &EventManager::getState() { return currentState; }

void EventManager::setLogicalScale(double scaleX, double scaleY)
{
    scaleX_ = scaleX;
    scaleY_ = scaleY;
}

void EventManager::clearNameInput() { nameBuffer_.clear(); }

std::string EventManager::getNameInputText() { return nameBuffer_; }

void EventManager::toLogical(int rawX, int rawY, int &x, int &y) const
{
    x = (int)(rawX / scaleX_);
    y = (int)(rawY / scaleY_);
}

void EventManager::gameStateReset()
{
    currentState.gameStart = false;
    currentState.gamePaused = false;
    currentState.gameOver = false;
    currentState.gameNameInput = false;
    currentState.gameLeaderboard = false;
    currentState.gameMenu = true;
}

namespace
{
void popUtf8Char(std::string &s)
{
    if (s.empty())
        return;
    size_t i = s.size() - 1;
    while (i > 0 && (static_cast<unsigned char>(s[i]) & 0xC0) == 0x80)
        --i;
    s.erase(i);
}
} // namespace

void EventManager::handleTextInputEvent(const SDL_Event &event)
{
    if (!currentState.gameNameInput)
        return;
    if (event.type == SDL_TEXTINPUT)
    {
        if (nameBuffer_.size() + strlen(event.text.text) <= 40)
            nameBuffer_ += event.text.text;
    }
    else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_BACKSPACE)
    {
        popUtf8Char(nameBuffer_);
    }
}

void EventManager::pollEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            currentState.gameMenu = false;
            currentState.gameNameInput = false;
            currentState.gameLeaderboard = false;
            currentState.gameWelcome = false;
            currentState.gamePaused = false;
            currentState.gamePreEnd = false;
            currentState.gameStart = false;
            currentState.appQuit = true;
            currentState.gameOver = true;
            return;
        }
        handleTextInputEvent(event);
    }

    int rawX, rawY;
    Uint32 buttons = SDL_GetMouseState(&rawX, &rawY);
    bool mouseLeftDown = (buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
    bool clicked = mouseLeftDown && !mouseLeftDownPrev_;
    mouseLeftDownPrev_ = mouseLeftDown;
    int mx, my;
    toLogical(rawX, rawY, mx, my);

    const Uint8 *keys = SDL_GetKeyboardState(nullptr);
    bool escDown = keys[SDL_SCANCODE_ESCAPE] != 0;
    bool escPressed = escDown && !escDownPrev_;
    escDownPrev_ = escDown;
    bool returnDown = keys[SDL_SCANCODE_RETURN] != 0;
    bool returnPressed = returnDown && !returnDownPrev_;
    returnDownPrev_ = returnDown;

    dispatch(mx, my, clicked, escPressed, returnPressed);
}

void EventManager::dispatch(int mx, int my, bool clicked, bool escPressed, bool returnPressed)
{
    if (currentState.gameMenu)
    {
        for (int i = 0; i < 3; i++)
        {
            if (!UILayout::PointInRect(mx, my, UILayout::MenuButtonRect(i)))
                continue;
            currentState.menuSelectedItem = (uint8_t)i;
            if (!clicked)
                continue;
            if (i == 0)
            {
                clearNameInput();
                currentState.gameNameInput = true;
                currentState.gameMenu = false;
            }
            else if (i == 1)
            {
                currentState.gameOver = true;
                currentState.gameMenu = false;
            }
            else
            {
                currentState.gameLeaderboard = true;
                currentState.gameMenu = false;
            }
        }
    }
    else if (currentState.gameNameInput)
    {
        if (returnPressed)
        {
            currentState.gameNameInput = false;
            currentState.gameWelcome = true;
        }
        else if (escPressed)
        {
            currentState.gameNameInput = false;
            currentState.gameMenu = true;
        }
    }
    else if (currentState.gameLeaderboard)
    {
        if (returnPressed || escPressed || clicked)
        {
            currentState.gameLeaderboard = false;
            currentState.gameMenu = true;
        }
    }
    else if (currentState.gameWelcome)
    {
        if (clicked)
        {
            currentState.gameStart = true;
            currentState.gameWelcome = false;
        }
    }
    else if (currentState.gamePreEnd)
    {
        if (returnPressed)
        {
            currentState.gamePreEnd = false;
        }
    }
    else if (currentState.gamePaused)
    {
        if (escPressed)
        {
            currentState.gamePaused = false;
        }
        else if (returnPressed)
        {
            currentState.gameStart = false;
            currentState.gamePaused = false;
            currentState.gameOver = true;
        }
    }
    else if (currentState.gameStart)
    {
        if (escPressed)
        {
            currentState.gamePaused = true;
        }
    }
}
