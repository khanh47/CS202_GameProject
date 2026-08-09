#include "Game/Objects/Enemy/ConcreteEnemy/Koopa.h"
#include "Game/Behaviours/Animatable.h"
#include "Game/Objects/Enemy/Enemy.h"
#include "Game/World/GameWorld.h"

Koopa::Koopa() : Enemy() {}

Koopa::Koopa(sf::Texture& texture, const std::string& animationSetId) : Enemy(texture, animationSetId) {
}

void Koopa::onUpdateVisuals(float deltaTime) {
    bool facingLeft = hasValidBody() && b2Body_GetLinearVelocity(_body->getId()).x > 0.f;
    if (auto* animatable = getBehaviour<Animatable>()) {
        animatable->updateVisualState(deltaTime, _hitboxPixels, facingLeft);
        animatable->setVisualScale({1.2f, 1.2f});
    }
}

void Koopa::onStomp() {
    if (_pendingDestroy || isDying()) {
        return;
    }

    const bool facingRight = !hasValidBody()
        || b2Body_GetLinearVelocity(_body->getId()).x >= 0.0f;
    if (_world) {
        _world->spawnKoopaShell(getPosition(), facingRight);
    }
    _pendingDestroy = true;
}

void Koopa::updateSimulation(const float &fixedDt) {
    if (isDying()) {
        if (auto* animatable = getBehaviour<Animatable>()) {
            animatable->setVisualScale({0.9f, 0.48f});
        }
    }
    Enemy::updateSimulation(fixedDt);
}