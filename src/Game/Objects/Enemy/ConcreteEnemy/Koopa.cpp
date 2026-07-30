#include "Game/Objects/Enemy/ConcreteEnemy/Koopa.h"

Koopa::Koopa() : Enemy() {}

Koopa::Koopa(sf::Texture& texture, const std::string& animationSetId) : Enemy(texture, animationSetId) {
}

void Koopa::onUpdateVisuals(float deltaTime) {
    bool facingLeft = hasValidBody() && b2Body_GetLinearVelocity(_body->getId()).x > 0.f;
    animatable->updateVisualState(deltaTime, _hitboxPixels, facingLeft);
    animatable->setVisualScale({1.2f, 1.2f});
}