#include "Game/Objects/Enemy/ConcreteEnemy/PiranhaPlant.h"
#include "Game/Behaviours/Animatable.h"
#include "box2d/box2d.h"

PiranhaPlant::PiranhaPlant() : Enemy() {}

PiranhaPlant::PiranhaPlant(sf::Texture& texture, const std::string& animationSetId) : Enemy(texture, animationSetId) {
}

void PiranhaPlant::updateSimulation(const float &fixedDt) {
    if (_isDying) {
        _deathTimer += fixedDt;
        if (_deathTimer >= 1.0f) {
            _pendingDestroy = true;
        }
        return;
    }

    if (auto* animatable = getBehaviour<Animatable>()) {
        animatable->playAnimation("bite");
    }

    b2Vec2 velocity = b2Body_GetLinearVelocity(_body->getId());
    velocity.x = 0.0f;
    b2Body_SetLinearVelocity(_body->getId(), velocity);
}

void PiranhaPlant::onStomp() {}

bool PiranhaPlant::canBeStomped() const {
    return false;
}
