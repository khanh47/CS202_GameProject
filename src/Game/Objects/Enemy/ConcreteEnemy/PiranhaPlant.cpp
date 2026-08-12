#include "Game/Objects/Enemy/ConcreteEnemy/PiranhaPlant.h"
#include "Game/Behaviours/Animatable.h"
#include "Physics/CollisionFilter.h"
#include <algorithm>
#include "box2d/box2d.h"

namespace {
constexpr float kTravelSpeedPixelsPerSecond = 72.0f;
constexpr float kHiddenDurationSeconds = 1.0f;
constexpr float kExposedDurationSeconds = 1.5f;
}

PiranhaPlant::PiranhaPlant() : Enemy() {}

PiranhaPlant::PiranhaPlant(sf::Texture& texture, const std::string& animationSetId) : Enemy(texture, animationSetId) {
}

void PiranhaPlant::setPipeTravel(float hiddenYPixels, float emergedYPixels) {
    _hasPipeTravel = true;
    _hiddenYPixels = hiddenYPixels;
    _emergedYPixels = emergedYPixels;
    beginPhase(PipePhase::Hidden);
    setPosition({getPosition().x, _hiddenYPixels});
}

void PiranhaPlant::onCreateBodyDef(b2BodyDef& def) {
    Enemy::onCreateBodyDef(def);
    def.gravityScale = 0.0f;
}

void PiranhaPlant::onCreateShapeDef(b2ShapeDef& def) {
    Enemy::onCreateShapeDef(def);
    // The plant overlaps the pipe while hidden, so it must not collide with
    // environment shapes. It remains hazardous to players and projectiles.
    def.filter.maskBits = CollisionFilter::PLAYER | CollisionFilter::FIREBALL
        | CollisionFilter::SHELL;
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
    velocity.y = 0.0f;

    if (!_hasPipeTravel) {
        b2Body_SetLinearVelocity(_body->getId(), velocity);
        return;
    }

    if (_pipePhase == PipePhase::Hidden || _pipePhase == PipePhase::Exposed) {
        _phaseTimer -= fixedDt;
        if (_phaseTimer <= 0.0f) {
            beginPhase(_pipePhase == PipePhase::Hidden
                ? PipePhase::Rising
                : PipePhase::Retracting);
        }
    }

    float targetY = getPosition().y;
    if (_pipePhase == PipePhase::Rising) {
        targetY = std::max(_emergedYPixels,
            getPosition().y - kTravelSpeedPixelsPerSecond * fixedDt);
        if (targetY <= _emergedYPixels) {
            beginPhase(PipePhase::Exposed);
        }
    } else if (_pipePhase == PipePhase::Retracting) {
        targetY = std::min(_hiddenYPixels,
            getPosition().y + kTravelSpeedPixelsPerSecond * fixedDt);
        if (targetY >= _hiddenYPixels) {
            beginPhase(PipePhase::Hidden);
        }
    }

    setPosition({getPosition().x, targetY});
    b2Body_SetLinearVelocity(_body->getId(), velocity);
}

void PiranhaPlant::onStomp() {}

bool PiranhaPlant::canBeStomped() const {
    return false;
}

void PiranhaPlant::beginPhase(PipePhase phase) {
    _pipePhase = phase;
    if (phase == PipePhase::Hidden) {
        _phaseTimer = kHiddenDurationSeconds;
    } else if (phase == PipePhase::Exposed) {
        _phaseTimer = kExposedDurationSeconds;
    } else {
        _phaseTimer = 0.0f;
    }
}
