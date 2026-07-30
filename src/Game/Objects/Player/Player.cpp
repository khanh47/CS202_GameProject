#include "Game/Objects/Player/Player.h"
#include "Game/World/GameWorld.h"
#include "Game/Objects/Player/State/NormalState.h"
#include "Game/Objects/Player/State/SuperState.h"
#include "Game/Objects/Player/State/FireState.h"
#include "Game/Objects/Player/State/MegaStateDecorator.h"
#include "Physics/PhysicsUnits.h"
#include "Game/Objects/Enemy/Enemy.h"
#include "Game/Objects/Item/FireFlower.h"
#include "ResourceManager.h"

#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Vector2.hpp>
#include <iostream>
#include <ostream>

Player::Player() : GameObject(), Animatable(), Damageable(100) {
    setState(std::make_unique<NormalState>());
}

Player::Player(sf::Texture &texture) : Player(texture, "mario") {
}

Player::Player(sf::Texture &texture, const std::string& animationSetId)
    : GameObject(), Animatable(), Damageable(100) {
    (void)texture;
    if (animationSetId == "fire_mario") {
        setState(std::make_unique<FireState>());
    } else {
        setState(std::make_unique<NormalState>());
    }
}

Player::~Player() = default;

void Player::setState(std::unique_ptr<PlayerState> newState) {
    if (!newState) return;

    if (_state) {
        _state->onExit(*this);
    }

    _state = std::move(newState);
    _state->onEnter(*this);
    _attackStrategy = _state->createAttackStrategy();

    // Apply texture and animation set dynamically from the current state
    const std::string& texAlias = _state->getTextureAlias();
    const std::string& animId = _state->getAnimationSetId();
    sf::Texture& tex = ResourceManager::getInstance().getTexture(texAlias);
    configureVisuals(tex, animId);
}

void Player::attack(GameWorld& world) {
    if (_attackStrategy) {
        _attackStrategy->executeAttack(*this, world);
    }
}

void Player::changeToNormalState() {
    setState(std::make_unique<NormalState>());
}

void Player::changeToSuperState() {
    setState(std::make_unique<SuperState>());
}

void Player::changeToFireState() {
    setState(std::make_unique<FireState>());
}

void Player::applyMegaState(float durationSeconds) {
    if (!_state) {
        _state = std::make_unique<NormalState>();
    }
    setState(std::make_unique<MegaStateDecorator>(std::move(_state), durationSeconds));
}

void Player::revertDecoratedState() {
    if (!_state) return;

    auto* decorator = dynamic_cast<PlayerStateDecorator*>(_state.get());
    if (decorator) {
        std::unique_ptr<PlayerState> unwrapped = decorator->unwrap();
        if (unwrapped) {
            setState(std::move(unwrapped));
        }
    }
}

void Player::updateSimulation(const float &fixedDt) {
    (void)fixedDt;

    if (!_body || !_body->isValid()) {
        return;
    }

    b2Vec2 velocity = b2Body_GetLinearVelocity(_body->getId());

    float moveSpeed = _baseMoveSpeed;
    float jumpSpeed = _baseJumpSpeed;
    if (_state) {
        moveSpeed *= _state->getMoveSpeedMultiplier();
        jumpSpeed *= _state->getJumpSpeedMultiplier();
    }

    if (isMovingLeft() && !isMovingRight()) {
        velocity.x = -moveSpeed;
    } else if (isMovingRight() && !isMovingLeft()) {
        velocity.x = moveSpeed;
    } else if (!isMovingLeft() && !isMovingRight()) {
        velocity.x = 0.f;
    }

    if (isJumping() && !isAirbone()) {
        velocity.y = -jumpSpeed;
        consumeGroundForJump();
    }


    b2Body_SetGravityScale(_body->getId(), 4.0f);
    if (isAirbone() || isJumping()) {
        if (velocity.y > 0) b2Body_SetGravityScale(_body->getId(), isJumping() ? 3.0f : 4.0f);
    }

    b2Body_SetLinearVelocity(_body->getId(), velocity);
}

void Player::finalizeSimulation(const float &fixedDt) {
    (void)fixedDt;

    if (isAirbone() || isJumping()) {
        playAnimation("jump");
    } else if (!isMovingLeft() && !isMovingRight()) {
        playAnimation("idle");
    } else {
        playAnimation("walk");
    }
}

void Player::onContact(GameObject& other) {
    if (auto* fireFlower = dynamic_cast<FireFlower*>(&other)) {
        if (_world) {
            startFireTransformation(*_world, 1.0f);
        }
        fireFlower->destroy();
    }
}

void Player::onCreateBodyDef(b2BodyDef& def) {
    def.type = b2_dynamicBody;
    def.motionLocks.angularZ = true;
}

void Player::onCreateShapeDef(b2ShapeDef& def) {
    def.density = 1.0f;
    def.material.friction = 0.0f;
    def.enablePreSolveEvents = true;

    // Category 0x0002 (Player), Mask 0x0001 | 0x0008 (Environment + Enemy)
    // Excludes Category 0x0004 (Fireball) so fireballs pass completely through Player
    def.filter.categoryBits = 0x0002;
    def.filter.maskBits = 0x0001 | 0x0008;
}

b2Polygon Player::makeHitbox(sf::Vector2f hitboxPixels) const {
    constexpr float cornerRadiusPixels = 4.0f;
    const float halfWidthPixels =
        std::max(0.0f, hitboxPixels.x * 0.5f - cornerRadiusPixels);
    const float halfHeightPixels =
        std::max(0.0f, hitboxPixels.y * 0.5f - cornerRadiusPixels);
    return b2MakeRoundedBox(
        PhysicsUnits::toMeters(halfWidthPixels),
        PhysicsUnits::toMeters(halfHeightPixels),
        PhysicsUnits::toMeters(cornerRadiusPixels)
    );
}

void Player::startFireTransformation(GameWorld& world, float duration) {
    if (_isTransforming) return;

    _isTransforming = true;
    _transformTimer = duration;
    _transformDuration = duration > 0.0f ? duration : 1.0f;

    // Stop movement inputs and clear horizontal physics velocity when transformation starts
    stopMoveLeft();
    stopMoveRight();
    stopJump();
    if (hasValidBody()) {
        const b2BodyId bodyId = _body->getId();
        b2Vec2 vel = b2Body_GetLinearVelocity(bodyId);
        vel.x = 0.0f;
        b2Body_SetLinearVelocity(bodyId, vel);
    }

    // Freeze physics simulation & world updates for the duration of transformation
    world.freeze(duration);

    // Switch visuals to transformation spritesheet & play transformation animation
    sf::Texture& transformTex = ResourceManager::getInstance().getTexture("mario_transform_spritesheet");
    configureVisuals(transformTex, "transform_fire");
    playAnimation("transform");
}

void Player::onUpdateVisuals(float deltaTime) {
    if (_isTransforming) {
        _transformTimer -= deltaTime;
        if (_transformTimer <= 0.0f) {
            _isTransforming = false;
            stopMoveLeft();
            stopMoveRight();
            stopJump();
            if (hasValidBody()) {
                const b2BodyId bodyId = _body->getId();
                b2Vec2 vel = b2Body_GetLinearVelocity(bodyId);
                vel.x = 0.0f;
                b2Body_SetLinearVelocity(bodyId, vel);
            }
            // Transformation finished -> transition into FireState
            changeToFireState();
            return;
        }

        // Gradually scale from 1.0x to 1.25x during transformation
        float progress = 1.0f - (_transformTimer / _transformDuration);
        progress = std::max(0.0f, std::min(1.0f, progress));
        float currentScale = 1.0f + 0.25f * progress;

        sf::Vector2f scaledHitbox = {_baseHitboxPixels.x * currentScale, _baseHitboxPixels.y * currentScale};
        updateHitboxSize(scaledHitbox);
        setVisualScale({1.8f, 1.0f});
        updateVisualState(deltaTime, scaledHitbox, isFacingLeft());
        return;
    }

    sf::Vector2f scaleMult{1.0f, 1.0f};
    if (_state) {
        _state->update(*this, deltaTime);
        // State replacement is deferred until update() returns, so a decorator
        // never destroys itself while one of its member functions is active.
        if (_state->isExpired()) {
            revertDecoratedState();
        }
        if (_state) {
            scaleMult = _state->getScaleMultiplier();
        }
    }

    sf::Vector2f scaledHitbox = {_baseHitboxPixels.x * scaleMult.x, _baseHitboxPixels.y * scaleMult.y};
    updateHitboxSize(scaledHitbox);
    setVisualScale({1.8f, 1.0f});
    updateVisualState(deltaTime, scaledHitbox, isFacingLeft());
}

void Player::onRenderVisual(sf::RenderTarget& target, const sf::Vector2f& position, float angleDegrees) {
    renderVisualState(target, position);
}

void Player::onHitboxRecreated() {
    resetGroundContacts();
}
