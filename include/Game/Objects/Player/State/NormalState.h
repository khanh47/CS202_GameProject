#pragma once

#include "Game/Objects/Player/State/PlayerState.h"

/**
 * @brief Concrete state representing standard small Mario.
 */
class NormalState : public PlayerState {
public:
    NormalState() = default;
    ~NormalState() override = default;

    std::string getStateName() const override { return "Normal"; }
    std::string getAnimationSetId() const override { return "mario"; }
    std::string getTextureAlias() const override { return "luigi_spritesheet"; }

    float getMoveSpeedMultiplier() const override { return 1.0f; }
    float getJumpSpeedMultiplier() const override { return 1.0f; }
    sf::Vector2f getScaleMultiplier() const override { return {1.0f, 1.0f}; }
    bool canShootFireballs() const override { return false; }
};
