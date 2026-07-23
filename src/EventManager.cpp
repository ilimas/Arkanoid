#include "EventManager.h"
#include "ResolutionPresets.h"
#include "UILayout.h"
#include <GLFW/glfw3.h>

GameState::GameState()
    : gameStart(false), gameWelcome(false), gamePaused(false), gameOver(false), gamePreEnd(false),
      gameMenu(true), gameNameInput(false), gameLeaderboard(false), gameSettings(false), appQuit(false),
      menuSelectedItem(0), settingsSelectedItem(0), pendingResolutionIndex(-1)
{
}

GameState &EventManager::getState() { return currentState; }

void EventManager::setViewport(int vx, int vy, int vw, int vh)
{
    vpX_ = vx;
    vpY_ = vy;
    vpW_ = vw > 0 ? vw : 1;
    vpH_ = vh > 0 ? vh : 1;
}

void EventManager::clearNameInput() { nameBuffer_.clear(); }

std::string EventManager::getNameInputText() { return nameBuffer_; }

void EventManager::toLogical(double rawX, double rawY, int &x, int &y) const
{
    x = (int)((rawX - vpX_) * (double)UILayout::ScreenW / vpW_);
    y = (int)((rawY - vpY_) * (double)UILayout::ScreenH / vpH_);
}

void EventManager::gameStateReset()
{
    currentState.gameStart = false;
    currentState.gamePaused = false;
    currentState.gameOver = false;
    currentState.gameNameInput = false;
    currentState.gameLeaderboard = false;
    currentState.gameSettings = false;
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

void appendUtf8(std::string &s, unsigned int codepoint)
{
    if (codepoint <= 0x7F)
    {
        s += (char)codepoint;
    }
    else if (codepoint <= 0x7FF)
    {
        s += (char)(0xC0 | (codepoint >> 6));
        s += (char)(0x80 | (codepoint & 0x3F));
    }
    else if (codepoint <= 0xFFFF)
    {
        s += (char)(0xE0 | (codepoint >> 12));
        s += (char)(0x80 | ((codepoint >> 6) & 0x3F));
        s += (char)(0x80 | (codepoint & 0x3F));
    }
    else
    {
        s += (char)(0xF0 | (codepoint >> 18));
        s += (char)(0x80 | ((codepoint >> 12) & 0x3F));
        s += (char)(0x80 | ((codepoint >> 6) & 0x3F));
        s += (char)(0x80 | (codepoint & 0x3F));
    }
}
} // namespace

void EventManager::attachWindow(GLFWwindow *window)
{
    window_ = window;
    glfwSetWindowUserPointer(window, this);
    glfwSetCharCallback(window, &EventManager::charCallback);
    glfwSetKeyCallback(window, &EventManager::keyCallback);
}

void EventManager::charCallback(GLFWwindow *window, unsigned int codepoint)
{
    static_cast<EventManager *>(glfwGetWindowUserPointer(window))->onChar(codepoint);
}

void EventManager::keyCallback(GLFWwindow *window, int key, int /*scancode*/, int action, int /*mods*/)
{
    if (key == GLFW_KEY_BACKSPACE && (action == GLFW_PRESS || action == GLFW_REPEAT))
        static_cast<EventManager *>(glfwGetWindowUserPointer(window))->onBackspace();
}

void EventManager::onChar(unsigned int codepoint)
{
    if (!currentState.gameNameInput)
        return;
    if (nameBuffer_.size() + 4 <= 40)
        appendUtf8(nameBuffer_, codepoint);
}

void EventManager::onBackspace()
{
    if (!currentState.gameNameInput)
        return;
    popUtf8Char(nameBuffer_);
}

void EventManager::pollEvents()
{
    glfwPollEvents();

    if (glfwWindowShouldClose(window_))
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

    double rawX, rawY;
    glfwGetCursorPos(window_, &rawX, &rawY);
    bool mouseLeftDown = glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    bool clicked = mouseLeftDown && !mouseLeftDownPrev_;
    mouseLeftDownPrev_ = mouseLeftDown;
    int mx, my;
    toLogical(rawX, rawY, mx, my);

    bool escDown = glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS;
    bool escPressed = escDown && !escDownPrev_;
    escDownPrev_ = escDown;
    bool returnDown = glfwGetKey(window_, GLFW_KEY_ENTER) == GLFW_PRESS ||
                       glfwGetKey(window_, GLFW_KEY_KP_ENTER) == GLFW_PRESS;
    bool returnPressed = returnDown && !returnDownPrev_;
    returnDownPrev_ = returnDown;

    dispatch(mx, my, clicked, escPressed, returnPressed);
}

void EventManager::dispatch(int mx, int my, bool clicked, bool escPressed, bool returnPressed)
{
    if (currentState.gameMenu)
    {
        for (int i = 0; i < 4; i++)
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
            else if (i == 2)
            {
                currentState.gameLeaderboard = true;
                currentState.gameMenu = false;
            }
            else
            {
                currentState.gameSettings = true;
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
    else if (currentState.gameSettings)
    {
        for (int i = 0; i < kResolutionPresetCount; i++)
        {
            if (!UILayout::PointInRect(mx, my, UILayout::MenuButtonRect(i)))
                continue;
            currentState.settingsSelectedItem = (uint8_t)i;
            if (clicked)
                currentState.pendingResolutionIndex = i;
        }
        if (returnPressed || escPressed)
        {
            currentState.gameSettings = false;
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
