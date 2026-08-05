#pragma once

#include <string>

enum class GameMode {
    Coop,
    Solo,
    Minigame
};

enum class MinigameMode {
    TwoPlayer,
    VsAi
};

class GameSettings {
public:
    static GameSettings& getInstance();

    GameMode gameMode = GameMode::Coop;
    MinigameMode minigameMode = MinigameMode::TwoPlayer;
    std::string player1Character = "mario";

    bool debugDrawGrid = false;
    bool debugDrawCoordinates = false;
    bool debugDrawHitbox = false;
    bool freeCameraMove = false;

private:
    GameSettings() = default;
    ~GameSettings() = default;
    
    // Delete copy/move constructors and assignment operators
    GameSettings(const GameSettings&) = delete;
    GameSettings& operator=(const GameSettings&) = delete;
    GameSettings(GameSettings&&) = delete;
    GameSettings& operator=(GameSettings&&) = delete;
};
