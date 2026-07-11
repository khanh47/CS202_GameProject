#pragma once

class GameSettings {
public:
    static GameSettings& getInstance();

    bool debugDrawGrid = true;
    bool debugDrawCoordinates = true;
    bool freeCameraMove = true;

private:
    GameSettings() = default;
    ~GameSettings() = default;
    
    // Delete copy/move constructors and assignment operators
    GameSettings(const GameSettings&) = delete;
    GameSettings& operator=(const GameSettings&) = delete;
    GameSettings(GameSettings&&) = delete;
    GameSettings& operator=(GameSettings&&) = delete;
};
