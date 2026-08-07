#include "Game/ScoreManager.h"
#include <cmath>

ScoreManager::ScoreManager() {
    _score = 0;
    _coins = 0;
    _lives = 3;
    _highScore = 0;
    _stompComboIndex = 0;
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

        case ScoreEventType::FlagpoleReached:
            pointsAwarded = (detail > 0) ? detail : 1000;
            displayText = std::to_string(pointsAwarded);
            break;

        case ScoreEventType::LostLive:
            // Reset stomp combo ladder when a life is lost
            _stompComboIndex = 0;
            _lives--;
            return;
    }

    if (pointsAwarded > 0) {
        addScore(pointsAwarded);
    }

    if (!displayText.empty() && (position.x != 0.f || position.y != 0.f)) {
        spawnFloatingText(position, displayText);
    }
}

void ScoreManager::update(float deltaTime) {
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
    std::string hudString = "MARIO        COINS        LIVES\n" + 
                            getFormattedScore() + "       " + getFormattedCoins() + "          x" + std::to_string(_lives);

    sf::Text hudText(font, hudString, 36);
    hudText.setPosition(hudPosition);
    hudText.setFillColor(sf::Color::White);
    hudText.setOutlineColor(sf::Color::Black);
    hudText.setOutlineThickness(2.0f);

    target.draw(hudText);
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
