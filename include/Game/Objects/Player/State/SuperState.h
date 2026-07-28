#pragma once

#include "Game/Objects/Player/State/PlayerState.h"

/**
 * @brief Concrete state representing Super Mario (enlarged size and boosted jump).
 */
class SuperState : public PlayerState {
public:
    SuperState() = default;
    ~SuperState() override = default;

    std::string getStateName() const override { return "Super"; }
    std::string getAnimationSetId() const override { return "mario"; }
    std::string getTextureAlias() const override { return "mario_spritesheet"; }

    float getMoveSpeedMultiplier() const override { return 1.1f; }
    float getJumpSpeedMultiplier() const override { return 1.15f; }
    sf::Vector2f getScaleMultiplier() const override { return {1.25f, 1.25f}; }
    bool canShootFireballs() const override { return false; }
};
