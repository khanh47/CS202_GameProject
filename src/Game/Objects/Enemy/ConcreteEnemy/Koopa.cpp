#include "Game/Objects/Enemy/ConcreteEnemy/Koopa.h"

Koopa::Koopa() : Enemy() {}

Koopa::Koopa(sf::Texture& texture, const std::string& animationSetId) : Enemy(texture, animationSetId) {
}

void Koopa::onUpdateVisuals(float deltaTime) {
    bool facingLeft = hasValidBody() && b2Body_GetLinearVelocity(_body->getId()).x > 0.f;
    animatable->updateVisualState(deltaTime, _hitboxPixels, facingLeft);
}

void Koopa::updateSimulation(const float &fixedDt) {
    b2Vec2 velocity = b2Body_GetLinearVelocity(_body->getId());
    velocity.x = _moveSpeed;
    b2Body_SetLinearVelocity(_body->getId(), velocity);
}