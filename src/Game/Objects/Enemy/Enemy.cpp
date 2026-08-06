#include "Game/Objects/Enemy/Enemy.h"
#include "Game/Behaviours/Animatable.h"
#include "Game/Objects/GameObject.h"
#include "Game/World/TerrainSeamFilter.h"
#include "Physics/PhysicsUnits.h"
#include "box2d/box2d.h"
#include <cmath>
#include <memory>

Enemy::Enemy() : GameObject() {
    addBehaviour<Animatable>();
    addBehaviour<Damageable>(50);
}

Enemy::Enemy(sf::Texture& texture) : Enemy() {
    if (auto* animatable = getBehaviour<Animatable>()) {
        animatable->configureVisuals(texture);
    }
}

Enemy::Enemy(sf::Texture &texture, const std::string& animationSetId) : Enemy() {
    if (auto* animatable = getBehaviour<Animatable>()) {
        animatable->configureVisuals(texture, animationSetId);
    }
}

Enemy::~Enemy() {
}

void Enemy::setSupportGrid(const TerrainSeamFilter* filter, float cellSize) {
    _supportGrid = filter;
    _supportCellSize = cellSize;
}

bool Enemy::isSupportedByGrid() const {
    if (!_supportGrid) {
        return false;
    }

    constexpr float probeBelowFeetPixels = 2.0f;
    const float feetY = getBodyPositionPixels().y + _hitboxPixels.y * 0.5f + probeBelowFeetPixels;

    return _supportGrid->isCellOccupied(probeColumn(), rowAt(feetY));
}

bool Enemy::isBlockedAhead() const {
    if (!_supportGrid) {
        return false;
    }

    return _supportGrid->isCellOccupied(probeColumn(), rowAt(getBodyPositionPixels().y));
}

int Enemy::probeColumn() const {
    if (!hasValidBody()) {
        return -1;
    }

    const b2Vec2 posMeters = b2Body_GetPosition(_body->getId());
    const sf::Vector2f posPx = PhysicsUnits::toPixels(posMeters);

    constexpr float probeForwardPixels = 2.0f;
    const float probeX = posPx.x + _moveDirection * (_hitboxPixels.x * 0.5f + probeForwardPixels);

    return static_cast<int>(std::floor(probeX / _supportCellSize));
}

int Enemy::rowAt(float pixelY) const {
    return static_cast<int>(std::floor((pixelY + _supportCellSize * 0.5f) / _supportCellSize));
}

void Enemy::turnAround() {
    _moveSpeed = -_moveSpeed;
    flipMoveDirection();
}

void Enemy::flipMoveDirection() {
    _moveDirection = -_moveDirection;
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
    if (auto* animatable = getBehaviour<Animatable>()) {
        animatable->updateVisualState(deltaTime, _hitboxPixels, facingLeft);
    }
}

void Enemy::onRenderVisual(sf::RenderTarget& target, const sf::Vector2f& position, float angleDegrees) {
    if (auto* animatable = getBehaviour<Animatable>()) {
        animatable->renderVisualState(target, position, angleDegrees);
    }
}

void Enemy::updateSimulation(const float &fixedDt) {
    (void)fixedDt;

    if (_supportGrid && (isBlockedAhead() || !isSupportedByGrid())) {
        turnAround();
    }

    b2Vec2 velocity = b2Body_GetLinearVelocity(_body->getId());
    velocity.x = _moveSpeed;
    b2Body_SetLinearVelocity(_body->getId(), velocity);
}

void Enemy::onDead() {
    if (auto* animatable = getBehaviour<Animatable>()) {
        animatable->playAnimation("dead");
    }
}