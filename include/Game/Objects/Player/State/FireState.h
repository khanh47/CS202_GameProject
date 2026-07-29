#pragma once

#include "Game/Objects/Player/State/PlayerState.h"
#include "Game/Behaviours/FireballAttackStrategy.h"

/**
 * @brief Concrete state representing Fire Mario, utilizing the white/red fire spritesheet
 * and enabling fireball attack capabilities.
 */
class FireState : public PlayerState {
public:
    FireState() = default;
    ~FireState() override = default;

    std::string getStateName() const override { return "Fire"; }
    std::string getAnimationSetId() const override { return "fire_mario"; }
    std::string getTextureAlias() const override { return "fire_mario_spritesheet"; }

    float getMoveSpeedMultiplier() const override { return 1.1f; }
    float getJumpSpeedMultiplier() const override { return 1.1f; }
    sf::Vector2f getScaleMultiplier() const override { return {1.25f, 1.25f}; }
    bool canShootFireballs() const override { return true; }
    std::unique_ptr<IAttackStrategy> createAttackStrategy() const override {
        return std::make_unique<FireballAttackStrategy>();
    }
};
