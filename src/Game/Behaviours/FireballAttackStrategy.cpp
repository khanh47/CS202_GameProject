#include "Game/Behaviours/FireballAttackStrategy.h"
#include "Game/Objects/Player/Player.h"
#include "Game/World/GameWorld.h"

void FireballAttackStrategy::executeAttack(Player& player, GameWorld& world) {
    sf::Vector2f playerPos = player.getPosition();
    const bool facingRight = !player.isFacingLeft();
    const int playerIndex = player.getCharacter() == "mario" ? 0 : 1;
    world.spawnFireball(playerPos, facingRight, playerIndex);
}
