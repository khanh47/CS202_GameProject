#include "Game/Objects/Player/State/StarManStateDecorator.h"
#include "Game/Objects/Player/State/SuperState.h"
#include "Game/Objects/Player/State/FireState.h"
#include "Game/Objects/Player/Player.h"

#include <string>

StarManStateDecorator::StarManStateDecorator(std::unique_ptr<PlayerState> wrappedState, float durationSeconds)
    : PlayerStateDecorator(std::move(wrappedState)),
      _remainingTime(durationSeconds) {
}

std::string StarManStateDecorator::getStateName() const {
    return "StarMan (" + PlayerStateDecorator::getStateName() + ")";
}

std::string StarManStateDecorator::getTextureAlias() const {
    // Derive character from the wrapped state's animation set id to pick
    // the correct rainbow spritesheet (already loaded by ResourceManager)
    std::string baseAnimId = PlayerStateDecorator::getAnimationSetId();
    bool isLuigi = baseAnimId.find("luigi") != std::string::npos;
    return isLuigi ? "rainbow_luigi_spritesheet" : "rainbow_mario_spritesheet";
}

float StarManStateDecorator::getMoveSpeedMultiplier() const {
    return PlayerStateDecorator::getMoveSpeedMultiplier() * 1.5f;
}

float StarManStateDecorator::getJumpSpeedMultiplier() const {
    return PlayerStateDecorator::getJumpSpeedMultiplier() * 1.2f;
}

void StarManStateDecorator::handleSuperMushroom(Player& player) {
    if (_wrappedState) {
        if (_wrappedState->getStateName() == "Normal") {
            _wrappedState = std::make_unique<SuperState>(player.getCharacter());
        }
    }
}

void StarManStateDecorator::handleFireFlower(Player& player) {
    if (_wrappedState) {
        if (_wrappedState->getStateName() != "Fire") {
            _wrappedState = std::make_unique<FireState>(player.getCharacter());
        }
    }
}

void StarManStateDecorator::handleSuperStar(Player& player) {
    (void)player;
    resetTimer(10.0f);
}

void StarManStateDecorator::update(Player& player, float dt) {
    PlayerStateDecorator::update(player, dt);
    _remainingTime -= dt;
}
