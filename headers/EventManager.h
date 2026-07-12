#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H

#include <SDL_events.h>
#include <cstdint>
#include <string>

struct GameState
{
    GameState();

    bool gameStart;
    bool gameWelcome;
    bool gamePaused;
    bool gameOver;
    bool gamePreEnd;
    bool gameMenu;
    bool gameNameInput;
    bool gameLeaderboard;
    bool appQuit;
    uint8_t menuSelectedItem;
};

// Polls SDL input once per frame from the main thread (no background threads
// or queues). Mouse position/buttons and the Escape/Enter keys are read as
// continuous device state and edge-detected here, the same way a modern
// engine's Input.GetKeyDown/GetMouseButtonDown works - SDL_GetMouseState and
// SDL_GetKeyboardState reflect the OS pointer/keyboard directly, which is more
// reliable across platforms than depending on discrete SDL_Event delivery for
// every single press. Free-form text entry (SDL_TEXTINPUT) has no polling
// equivalent, so the name-input screen still reads that off the event queue.
class EventManager
{
  public:
    EventManager() = default;
    void gameStateReset();
    void pollEvents();
    GameState &getState();
    void setLogicalScale(double scaleX, double scaleY);
    void clearNameInput();
    std::string getNameInputText();

  private:
    void handleTextInputEvent(const SDL_Event &event);
    void dispatch(int mx, int my, bool clicked, bool escPressed, bool returnPressed);
    void toLogical(int rawX, int rawY, int &x, int &y) const;

    GameState currentState{};
    double scaleX_{1.0};
    double scaleY_{1.0};
    std::string nameBuffer_;

    bool mouseLeftDownPrev_ = false;
    bool escDownPrev_ = false;
    bool returnDownPrev_ = false;
};

#endif
