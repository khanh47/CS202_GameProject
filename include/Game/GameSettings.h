#include <SFML/Window/Keyboard.hpp>
#include <string>
#include "Game/UserInput/Action.h"

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

    // Player controls (keybindings)
    sf::Keyboard::Key keyMoveLeft = sf::Keyboard::Key::A;
    sf::Keyboard::Key keyMoveRight = sf::Keyboard::Key::D;
    sf::Keyboard::Key keyJump = sf::Keyboard::Key::W;
    sf::Keyboard::Key keyAttack = sf::Keyboard::Key::X;
    sf::Keyboard::Key keyInteract = sf::Keyboard::Key::LShift;

    sf::Keyboard::Key getKeyForAction(ActionType action) const;
    void setKeyForAction(ActionType action, sf::Keyboard::Key key);

    static std::string keyToString(sf::Keyboard::Key key);

private:
    GameSettings() = default;
    ~GameSettings() = default;
    
    // Delete copy/move constructors and assignment operators
    GameSettings(const GameSettings&) = delete;
    GameSettings& operator=(const GameSettings&) = delete;
    GameSettings(GameSettings&&) = delete;
    GameSettings& operator=(GameSettings&&) = delete;
};
