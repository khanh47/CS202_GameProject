#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <variant>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cmath>

// Enum representing score-triggering events in Mario
enum class ScoreEventType {
    CoinCollected,
    EnemyStomped,
    BlockBroken,
    PowerupCollected,
    OneUpCollected,
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

    // Render fixed HUD overlay (Score, Coins, Lives, Time)
    void renderHUD(sf::RenderTarget& target, const sf::Font& font, sf::Vector2f hudPosition = {20.f, 20.f}) const;

    // Formatted HUD getters
    std::string getFormattedScore() const;
    std::string getFormattedCoins() const;
    std::string getFormattedTime() const;

    // Time Management
    void setTimeRemaining(float seconds) { _timeRemaining = std::max(0.0f, seconds); }
    void setInitialTime(float seconds) { _initialTime = std::max(0.0f, seconds); }
    void resetTime(float seconds = -1.0f) { _timeRemaining = (seconds >= 0.0f) ? seconds : _initialTime; _timePaused = false; }
    float getTimeRemaining() const { return _timeRemaining; }
    int getIntTimeRemaining() const { return static_cast<int>(std::max(0.0f, std::ceil(_timeRemaining))); }
    bool isTimeUp() const { return _timeRemaining <= 0.0f; }
    bool isTimePaused() const { return _timePaused; }
    void setTimePaused(bool paused) { _timePaused = paused; }

    // Convert level time left into score bonus
    int convertRemainingTimeToScore(int pointsPerSecond = 50);
    int convertRemainingTimeToScore(int secondsLeft, int pointsPerSecond);

    // Getters & Setters
    int getScore() const { return _score; }
    int getCoins() const { return _coins; }
    int getLives() const { return _lives; }
    int getHighScore() const { return _highScore; }
    void setLives(int lives) { _lives = lives; }
    void restoreState(int score, int coins, int lives, int highScore, float timeRemaining = 400.0f) {
        _score = std::max(score, 0);
        _coins = std::max(coins, 0);
        _lives = std::max(lives, 0);
        _highScore = std::max({0, highScore, _score});
        _timeRemaining = std::max(0.0f, timeRemaining);
        _timePaused = false;
        _stompComboIndex = 0;
        _floatingTexts.clear();
    }

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

    float _timeRemaining = 400.0f;
    float _initialTime = 400.0f;
    bool _timePaused = false;

    // Super Mario Bros stomp reward sequence
    const std::vector<std::variant<int, std::string>> _stompSequence = {
        100, 200, 400, 800, 1000, 2000, 4000, 8000, "1UP"
    };

    std::vector<FloatingText> _floatingTexts;
};
