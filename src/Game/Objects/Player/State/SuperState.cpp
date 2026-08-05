#include "Game/Objects/Player/State/SuperState.h"
#include "Game/Objects/Player/Player.h"

#include <utility>

SuperState::SuperState(std::string character)
    : _character(std::move(character)) {}

void SuperState::handleSuperMushroom(Player& player) {
    (void)player;
}

void SuperState::handleFireFlower(Player& player) {
    player.startTransformation(Player::TransformTarget::Fire);
}

void SuperState::handleSuperStar(Player& player) {
    player.startTransformation(Player::TransformTarget::StarMan);
}
