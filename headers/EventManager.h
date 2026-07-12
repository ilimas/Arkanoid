#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H

#include <SDL_events.h>
#include <atomic>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <optional>
#include <string>
#include <thread>

struct GameState
{
    GameState();

    std::atomic<bool> gameStart;
    std::atomic<bool> gameWelcome;
    std::atomic<bool> gamePaused;
    std::atomic<bool> gameOver;
    std::atomic<bool> gamePreEnd;
    std::atomic<bool> gameMenu;
    std::atomic<bool> gameNameInput;
    std::atomic<bool> gameLeaderboard;
    std::atomic<bool> appQuit;
    std::atomic_uint8_t menuSelectedItem;
};

class EventManager
{
  public:
    EventManager() = default;
    void gameStateReset();
    void waitForEvent();
    std::optional<SDL_Event> popEvent();
    bool tryPopEvent(SDL_Event &out);
    void clearQueue();
    void startCaptureEvents();
    void stopCapture();
    GameState &getState();
    void setLogicalScale(double scaleX, double scaleY);
    void clearNameInput();
    std::string getNameInputText();

  private:
    void acceptEventsWorker(std::stop_token stopToken);
    void processEventsWorker(std::stop_token stopToken);
    void getLogicalMousePos(int &x, int &y);

    GameState currentState{};
    std::deque<SDL_Event> queue_;
    std::mutex mutex_;
    std::condition_variable condition_;
    std::jthread pollThread;
    std::jthread eventThread;
    double scaleX_{1.0};
    double scaleY_{1.0};
    std::mutex nameMutex_;
    std::string nameBuffer_;
};

#endif
