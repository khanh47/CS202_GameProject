#include "Game/Objects/Player/Player.h"
#include "Game/Objects/Player/State/NormalState.h"
#include "Game/Objects/Player/State/SuperState.h"
#include "Game/Objects/Player/State/FireState.h"
#include "Game/Objects/Player/State/MegaStateDecorator.h"
#include "ResourceManager.h"

#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Vector2.hpp>

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
    b2Body_SetGravityScale(_body->getId(), 4.0f);

    float moveSpeed = _baseMoveSpeed;
    float jumpSpeed = _baseJumpSpeed;
    if (_state) {
        moveSpeed *= _state->getMoveSpeedMultiplier();
        jumpSpeed *= _state->getJumpSpeedMultiplier();
    }

    if (isJumping()) {
        velocity.y = -jumpSpeed;
        playAnimation("jump");
    }
    else if (isMovingLeft() && !isMovingRight()) {
        velocity.x = -moveSpeed;
        playAnimation("walk");
    }
    else if (isMovingRight() && !isMovingLeft()) {
        velocity.x = moveSpeed;
        playAnimation("walk");
    }
    else if (!isMovingLeft() && !isMovingRight()) {
        velocity.x = 0.f;
        playAnimation("idle");
    } 

    b2Body_SetLinearVelocity(_body->getId(), velocity);
}

void Player::onCreateBodyDef(b2BodyDef& def) {
    def.type = b2_dynamicBody;
    def.motionLocks.angularZ = true;
}

void Player::onCreateShapeDef(b2ShapeDef& def) {
    def.density = 1.0f;
    def.material.friction = 0.0f;
    
    // Category 0x0002 (Player), Mask 0x0001 | 0x0008 (Environment + Enemy)
    // Excludes Category 0x0004 (Fireball) so fireballs pass completely through Player
    def.filter.categoryBits = 0x0002;
    def.filter.maskBits = 0x0001 | 0x0008;
}

void Player::onUpdateVisuals(float deltaTime) {
    sf::Vector2f scaleMult{1.0f, 1.0f};
    if (_state) {
        _state->update(*this, deltaTime);
        scaleMult = _state->getScaleMultiplier();
    }

    sf::Vector2f scaledHitbox = {_hitboxPixels.x * scaleMult.x, _hitboxPixels.y * scaleMult.y};
    updateVisualState(deltaTime, scaledHitbox, isFacingLeft());
}

void Player::onRenderVisual(sf::RenderTarget& target, const sf::Vector2f& position, float angleDegrees) {
    renderVisualState(target, position);
}
