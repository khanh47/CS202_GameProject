#include "Game/Objects/Enemy/Enemy.h"

Enemy::Enemy() : GameObject(), Animatable(), Damageable(50) {
}

Enemy::Enemy(sf::Texture& texture) : GameObject(), Animatable(), Damageable(50) {
    configureVisuals(texture);
}

Enemy::~Enemy() {
}

void Enemy::onCreateBodyDef(b2BodyDef& def) {
    def.type = b2_dynamicBody;
}

void Enemy::onUpdateVisuals(float deltaTime) {
    updateVisualState(deltaTime, _hitboxPixels);
}

void Enemy::onRenderVisual(sf::RenderTarget& target, const sf::Vector2f& position, float angleDegrees) {
    renderVisualState(target, position, angleDegrees);
}
