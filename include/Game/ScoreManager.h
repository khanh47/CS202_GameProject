#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <variant>
#include <iomanip>
#include <sstream>
#include <algorithm>

// Enum representing score-triggering events in Mario
enum class ScoreEventType {
    CoinCollected,
    EnemyStomped,
    BlockBroken,
    PowerupCollected,
    FlagpoleReached,
    MarioLanded,
    LostLive,
    CoinBlockTouched,
    MegaCoinCollected      // Resets the airborne stomp combo ladder
};

// Visual floating score text (+200, 1UP, etc.) in world space
struct FloatingText {
    sf::Vector2f position;
    std::string text;
    float alpha = 1.0f;
    float velocityY = -40.0f; // Floating upwards speed (pixels/sec)
    
    void update(float deltaTime) {
        position.y += velocityY * deltaTime;
        alpha -= 0.8f * deltaTime; // Fade out over ~1.25s
        if (alpha < 0.0f) alpha = 0.0f;
    }

    bool isDead() const {
        return alpha <= 0.0f;
    }
};

class ScoreManager {
public:
    ScoreManager();
    ~ScoreManager() = default;

    // Primary entry point to trigger score events
    void handleEvent(ScoreEventType event, sf::Vector2f position = {0.f, 0.f}, int detail = 0);

    // Call in game loop update step
    void update(float deltaTime);

    // Render floating score numbers in world coordinates
    void renderFloatingTexts(sf::RenderTarget& target, const sf::Font& font) const;

    // Render fixed HUD overlay (Score, Coins, Lives)
    void renderHUD(sf::RenderTarget& target, const sf::Font& font, sf::Vector2f hudPosition = {20.f, 20.f}) const;

    // Convert level time left into score bonus
    int convertRemainingTimeToScore(int secondsLeft, int pointsPerSecond = 50);

    // Formatted HUD getters
    std::string getFormattedScore() const;
    std::string getFormattedCoins() const;

    // Getters & Setters
    int getScore() const { return _score; }
    int getCoins() const { return _coins; }
    int getLives() const { return _lives; }
    int getHighScore() const { return _highScore; }
    void setLives(int lives) { _lives = lives; }

    const std::vector<FloatingText>& getFloatingTexts() const { return _floatingTexts; }

private:
    void addScore(int amount);
    void addCoins(int amount);
    void spawnFloatingText(sf::Vector2f position, const std::string& text);

    int _score = 0;
    int _coins = 0;
    int _lives = 3;
    int _highScore = 0;
    int _stompComboIndex = 0;

    // Super Mario Bros stomp reward sequence
    const std::vector<std::variant<int, std::string>> _stompSequence = {
        100, 200, 400, 800, 1000, 2000, 4000, 8000, "1UP"
    };

    std::vector<FloatingText> _floatingTexts;
};
