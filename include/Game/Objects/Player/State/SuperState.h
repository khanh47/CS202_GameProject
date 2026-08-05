#pragma once

#include <string>

#include "Game/Objects/Player/State/PlayerState.h"

/**
 * @brief Concrete state representing Super player (enlarged size and boosted jump).
 */
class SuperState : public PlayerState {
public:
    explicit SuperState(std::string character = "mario");
    ~SuperState() override = default;

    std::string getStateName() const override { return "Super"; }
    std::string getAnimationSetId() const override { return _character; }
    std::string getTextureAlias() const override { return _character + "_spritesheet"; }

    float getMoveSpeedMultiplier() const override { return 1.1f; }
    float getJumpSpeedMultiplier() const override { return 1.15f; }
    sf::Vector2f getScaleMultiplier() const override { return {1.25f, 1.25f}; }
    bool canShootFireballs() const override { return false; }

private:
    std::string _character;
};
