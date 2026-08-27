#include "Game/ScoreManager.h"
#include "Game/Objects/Block/CoinBlock.h"
#include <cmath>

ScoreManager::ScoreManager() {
    _score = 0;
    _coins = 0;
    _lives = 3;
    _highScore = 0;
    _stompComboIndex = 0;
    _timeRemaining = 400.0f;
    _initialTime = 400.0f;
    _timePaused = false;
}

void ScoreManager::handleEvent(ScoreEventType event, sf::Vector2f position, int detail) {
    int pointsAwarded = 0;
    std::string displayText = "";

    switch (event) {
        case ScoreEventType::CoinCollected:
            addCoins(1);
            pointsAwarded = 200;
            displayText = "200";
            break;

        case ScoreEventType::MegaCoinCollected:
            // Reset stomp combo ladder when a Mega Coin is collected
            _stompComboIndex = 0;
            // addCoins(1);
            pointsAwarded = 1000; // Mega Coin gives more points
            displayText = "1000";
            break;

        case ScoreEventType::EnemyStomped: {
            size_t index = std::min(static_cast<size_t>(_stompComboIndex), _stompSequence.size() - 1);
            auto reward = _stompSequence[index];

            if (std::holds_alternative<std::string>(reward)) {
                _lives++;
                displayText = std::get<std::string>(reward); // "1UP"
            } else {
                pointsAwarded = std::get<int>(reward);
                displayText = std::to_string(pointsAwarded);
            }

            // Increment combo count for consecutive airborne stomps
            _stompComboIndex++;
            break;
        }

        case ScoreEventType::MarioLanded:
            // Touch ground -> Reset stomp combo ladder
            _stompComboIndex = 0;
            return;

        case ScoreEventType::BlockBroken:
            pointsAwarded = 50;
            displayText = "50";
            break;

        case ScoreEventType::PowerupCollected:
            pointsAwarded = 1000;
            displayText = "1000";
            break;

        case ScoreEventType::OneUpCollected:
            _lives++;
            displayText = "1UP";
            break;

        case ScoreEventType::FlagpoleReached: {
            int flagpolePoints = (detail > 0) ? detail : 1000;
            pointsAwarded = flagpolePoints;
            displayText = std::to_string(pointsAwarded);
            _timePaused = true;
            break;
        }

        case ScoreEventType::LostLive:
            // Reset stomp combo ladder when a life is lost
            _stompComboIndex = 0;
            _lives--;
            break;

        case ScoreEventType::CoinBlockTouched:
            // Reset stomp combo ladder when a coin block is touched
            addCoins(1);
            pointsAwarded = 200;
            displayText = "200";
            break;
    }

    if (pointsAwarded > 0) {
        addScore(pointsAwarded);
    }

    if (!displayText.empty() && (position.x != 0.f || position.y != 0.f)) {
        spawnFloatingText(position, displayText);
    }
}

void ScoreManager::update(float deltaTime) {
    // Update level timer
    if (!_timePaused && _timeRemaining > 0.0f) {
        _timeRemaining -= deltaTime;
        if (_timeRemaining < 0.0f) {
            _timeRemaining = 0.0f;
        }
    }

    for (auto& popup : _floatingTexts) {
        popup.update(deltaTime);
    }

    _floatingTexts.erase(
        std::remove_if(_floatingTexts.begin(), _floatingTexts.end(),
            [](const FloatingText& text) { return text.isDead(); }),
        _floatingTexts.end()
    );
}

void ScoreManager::renderFloatingTexts(sf::RenderTarget& target, const sf::Font& font) const {
    for (const auto& popup : _floatingTexts) {
        sf::Text text(font, popup.text, 28);
        text.setPosition(popup.position);
        
        std::uint8_t alphaByte = static_cast<std::uint8_t>(std::clamp(popup.alpha * 255.0f, 0.0f, 255.0f));
        text.setFillColor(sf::Color(255, 255, 255, alphaByte));
        text.setOutlineColor(sf::Color(0, 0, 0, alphaByte));
        text.setOutlineThickness(1.0f);

        target.draw(text);
    }
}

void ScoreManager::renderHUD(sf::RenderTarget& target, const sf::Font& font, sf::Vector2f hudPosition) const {
    auto drawColumn = [&](const std::string& header, const std::string& value, float x) {
        std::string colText = header + "\n" + value;
        sf::Text text(font, colText, 36);
        text.setPosition({x, hudPosition.y});
        text.setFillColor(sf::Color::White);
        text.setOutlineColor(sf::Color::Black);
        text.setOutlineThickness(2.0f);
        target.draw(text);
    };

    // Fixed absolute column positions: Each section stays stuck in place independently
    drawColumn("MARIO", getFormattedScore(), hudPosition.x);
    drawColumn("COINS", getFormattedCoins(), hudPosition.x + 360.0f);
    drawColumn("LIVES", "x" + std::to_string(_lives), hudPosition.x + 700.0f);
    drawColumn("TIME",  getFormattedTime(), hudPosition.x + 1040.0f);
}

int ScoreManager::convertRemainingTimeToScore(int pointsPerSecond) {
    int secondsLeft = getIntTimeRemaining();
    int bonusScore = secondsLeft * pointsPerSecond;
    addScore(bonusScore);
    _timeRemaining = 0.0f;
    return bonusScore;
}

int ScoreManager::convertRemainingTimeToScore(int secondsLeft, int pointsPerSecond) {
    int bonusScore = secondsLeft * pointsPerSecond;
    addScore(bonusScore);
    return bonusScore;
}

void ScoreManager::addScore(int amount) {
    _score += amount;
    if (_score > _highScore) {
        _highScore = _score;
    }
}

void ScoreManager::addCoins(int amount) {
    _coins += amount;
    if (_coins >= 100) {
        _coins -= 100;
        _lives++;
    }
}

void ScoreManager::spawnFloatingText(sf::Vector2f position, const std::string& text) {
    _floatingTexts.push_back({ position, text, 1.0f, -40.0f });
}

std::string ScoreManager::getFormattedScore() const {
    std::ostringstream ss;
    ss << std::setw(6) << std::setfill('0') << _score;
    return ss.str();
}

std::string ScoreManager::getFormattedCoins() const {
    std::ostringstream ss;
    ss << "x" << std::setw(2) << std::setfill('0') << _coins;
    return ss.str();
}

std::string ScoreManager::getFormattedTime() const {
    std::ostringstream ss;
    ss << std::setw(3) << std::setfill('0') << getIntTimeRemaining();
    return ss.str();
}
