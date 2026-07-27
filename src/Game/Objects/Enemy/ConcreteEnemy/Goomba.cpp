#include "Game/Objects/Enemy/ConcreteEnemy/Goomba.h"

Goomba::Goomba() : Enemy() {}

Goomba::Goomba(sf::Texture& texture, const std::string& animationSetId) : Enemy(texture, animationSetId) {
}

void Goomba::updateSimulation(const float &fixedDt) {
    b2Vec2 velocity = b2Body_GetLinearVelocity(_body->getId());
    velocity.x = _moveSpeed;
    b2Body_SetLinearVelocity(_body->getId(), velocity);
}