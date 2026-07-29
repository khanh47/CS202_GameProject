#include "Game/Behaviours/FireballAttackStrategy.h"
#include "Game/Objects/Player/Player.h"
#include "Game/World/GameWorld.h"

void FireballAttackStrategy::executeAttack(Player& player, GameWorld& world) {
    sf::Vector2f playerPos = player.getPosition();
    const bool facingRight = !player.isFacingLeft();
    world.spawnFireball(playerPos, facingRight);
}
