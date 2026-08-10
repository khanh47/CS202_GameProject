#include "Game/Objects/Player/State/FireState.h"
#include "Game/Behaviours/Animatable.h"
#include "Game/Behaviours/Invincible.h"
#include "Game/Objects/Player/Player.h"

#include <utility>

FireState::FireState(std::string character)
    : _character(std::move(character)) {}

void FireState::handleSuperMushroom(Player& player) {
    (void)player;
}

void FireState::handleFireFlower(Player& player) {
    (void)player;
}

void FireState::handleSuperStar(Player& player) {
    player.startTransformation(Player::TransformTarget::StarMan);
}

void FireState::handleEnemy(Player& player) {
    auto* animatable = player.getBehaviour<Animatable>();
    animatable->playAnimation("hit");
    player.startTransformation(Player::TransformTarget::Normal, 0);
    player.addBehaviour<Invincible>(2.0f);
}