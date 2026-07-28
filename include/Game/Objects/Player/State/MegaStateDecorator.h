#pragma once

#include "Game/Objects/Player/State/PlayerStateDecorator.h"

/**
 * @brief Decorator providing temporary Mega Mushroom status (giant size, speed boost)
 * for a limited duration, automatically reverting to the base state upon expiration.
 */
class MegaStateDecorator : public PlayerStateDecorator {
public:
    MegaStateDecorator(std::unique_ptr<PlayerState> wrappedState, float durationSeconds = 5.0f);
    ~MegaStateDecorator() override = default;

    std::string getStateName() const override;
    float getMoveSpeedMultiplier() const override;
    float getJumpSpeedMultiplier() const override;
    sf::Vector2f getScaleMultiplier() const override;

    void update(Player& player, float dt) override;

private:
    float _remainingTime;
    float _initialDuration;
};
