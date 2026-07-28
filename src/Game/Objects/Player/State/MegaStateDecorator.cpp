#include "Game/Objects/Player/State/MegaStateDecorator.h"
#include "Game/Objects/Player/Player.h"

MegaStateDecorator::MegaStateDecorator(std::unique_ptr<PlayerState> wrappedState, float durationSeconds)
    : PlayerStateDecorator(std::move(wrappedState)),
      _remainingTime(durationSeconds),
      _initialDuration(durationSeconds) {
}

std::string MegaStateDecorator::getStateName() const {
    return "Mega (" + PlayerStateDecorator::getStateName() + ")";
}

float MegaStateDecorator::getMoveSpeedMultiplier() const {
    return PlayerStateDecorator::getMoveSpeedMultiplier() * 1.5f;
}

float MegaStateDecorator::getJumpSpeedMultiplier() const {
    return PlayerStateDecorator::getJumpSpeedMultiplier() * 1.2f;
}

sf::Vector2f MegaStateDecorator::getScaleMultiplier() const {
    sf::Vector2f baseScale = PlayerStateDecorator::getScaleMultiplier();
    return {baseScale.x * 2.0f, baseScale.y * 2.0f};
}

void MegaStateDecorator::update(Player& player, float dt) {
    PlayerStateDecorator::update(player, dt);

    _remainingTime -= dt;
    if (_remainingTime <= 0.0f) {
        // Revert decorator back to inner state when effect duration expires
        player.revertDecoratedState();
    }
}
