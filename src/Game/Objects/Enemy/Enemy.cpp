#include "Game/Objects/Enemy/Enemy.h"
#include "Game/Behaviours/Animatable.h"
#include "Game/Behaviours/Damageable.h"
#include "box2d/box2d.h"
#include <iostream>
#include <memory>
#include <ostream>

Enemy::Enemy() : GameObject() {
    animatable = std::make_unique<Animatable>();
    damageable = std::make_unique<Damageable>(50);
}

Enemy::Enemy(sf::Texture& texture) : Enemy() {
    animatable->configureVisuals(texture);
}

Enemy::Enemy(sf::Texture &texture, const std::string& animationSetId) : Enemy() {
    animatable->configureVisuals(texture, animationSetId);
}

Enemy::~Enemy() {
}

void Enemy::onCreateBodyDef(b2BodyDef& def) {
    def.type = b2_dynamicBody;
    def.motionLocks.angularZ = true;
}

void Enemy::onCreateShapeDef(b2ShapeDef& def) {
    def.density = 1.0f;
    def.material.friction = 0.0f;
    
    // Category 0x0008 (Enemy)
    def.filter.categoryBits = 0x0008;
}

void Enemy::onUpdateVisuals(float deltaTime) {
    bool facingLeft = hasValidBody() && b2Body_GetLinearVelocity(_body->getId()).x < 0.f;
    animatable->updateVisualState(deltaTime, _hitboxPixels, facingLeft);
}

void Enemy::onRenderVisual(sf::RenderTarget& target, const sf::Vector2f& position, float angleDegrees) {
    animatable->renderVisualState(target, position, angleDegrees);
}

void Enemy::updateSimulation(const float &fixedDt) {
    b2Vec2 velocity = b2Body_GetLinearVelocity(_body->getId());
    velocity.x = _moveSpeed;
    b2Body_SetLinearVelocity(_body->getId(), velocity);
}