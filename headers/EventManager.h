#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H

#include <cstdint>
#include <string>

struct GLFWwindow;

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
    bool gameSettings;
    bool appQuit;
    uint8_t menuSelectedItem;
    uint8_t settingsSelectedItem;
    int pendingResolutionIndex; // set by dispatch() on a settings-screen click, consumed by Game::run()
};

// Polls GLFW input once per frame from the main thread (no background threads
// or queues). Mouse position/buttons and the Escape/Enter keys are read as
// continuous device state and edge-detected here, the same way a modern
// engine's Input.GetKeyDown/GetMouseButtonDown works. Free-form Unicode text
// entry (for the name-input screen) has no polling equivalent, so it's
// captured via GLFW's character/key callbacks instead - see attachWindow().
class EventManager
{
  public:
    EventManager() = default;
    void gameStateReset();
    // Registers this instance's GLFW callbacks (char/key, for text input) on
    // the window and remembers it for pollEvents(). Call once, after the
    // window is created.
    void attachWindow(GLFWwindow *window);
    void pollEvents();
    GameState &getState();
    // Matches GLRenderer's letterboxed viewport (vx,vy = top-left offset in
    // window pixels, vw,vh = its size) so mouse hit-testing lines up with
    // where things are actually drawn regardless of window aspect ratio.
    void setViewport(int vx, int vy, int vw, int vh);
    void clearNameInput();
    std::string getNameInputText();

  private:
    static void charCallback(GLFWwindow *window, unsigned int codepoint);
    static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
    void onChar(unsigned int codepoint);
    void onBackspace();
    void dispatch(int mx, int my, bool clicked, bool escPressed, bool returnPressed);
    void toLogical(double rawX, double rawY, int &x, int &y) const;

    GLFWwindow *window_{nullptr};
    GameState currentState{};
    int vpX_{0}, vpY_{0}, vpW_{1}, vpH_{1};
    std::string nameBuffer_;

    bool mouseLeftDownPrev_ = false;
    bool escDownPrev_ = false;
    bool returnDownPrev_ = false;
};

#endif
