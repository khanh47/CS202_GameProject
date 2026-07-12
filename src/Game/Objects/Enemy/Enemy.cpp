#include "Game/Objects/Enemy/Enemy.h"

Enemy::Enemy() : GameObject(), Damageable(50) {
}

Enemy::Enemy(sf::Texture& texture) : GameObject(), Damageable(50) {
    _sprite = sf::Sprite(texture);
}

Enemy::~Enemy() {
}

void Enemy::onCreateBodyDef(b2BodyDef& def) {
    def.type = b2_dynamicBody;
}
