#include "Game/Objects/Player/Player.h"
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Vector2.hpp>
#include <iostream>
#include <ostream>

Player::Player() : GameObject(), Animatable(), Damageable(100) {
}

Player::Player(sf::Texture &texture) : Player(texture, "mario") {
}

/*
Gia Khanh: we shold remove the constructors with texture when implementing the concrete
objects (such as Mario, Luigi), and let the Animatable construct the texture:v
like
Mario::Mario(): GameObject(), Animatable(marioTexture, "mario"), Damagable(100) {}
*/ 

Player::Player(sf::Texture &texture, const std::string& animationSetId) : GameObject(), Animatable(), Damageable(100) {
    configureVisuals(texture, animationSetId);
}

Player::~Player() {
}

void Player::updateSimulation(const float &fixedDt) {
    (void)fixedDt;

    if (!_body || !_body->isValid()) {
        return;
    }

    b2Vec2 velocity = b2Body_GetLinearVelocity(_body->getId());

    if (isMovingLeft() && !isMovingRight()) {
        velocity.x = -_moveSpeed;
    } else if (isMovingRight() && !isMovingLeft()) {
        velocity.x = _moveSpeed;
    } else if (!isMovingLeft() && !isMovingRight()) {
        velocity.x = 0.f;
    }

    if (isJumping() && !isAirbone()) {
        velocity.y = -_jumpSpeed;
    }


    b2Body_SetGravityScale(_body->getId(), 4.0f);
    if (isAirbone() || isJumping()) {
        if (velocity.y > 0) b2Body_SetGravityScale(_body->getId(), isJumping() ? 3.0f : 4.0f);
        playAnimation("jump");
    } else {
        if (!isMovingLeft() && !isMovingRight()) playAnimation("idle");
        else playAnimation("walk");
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
}

void Player::onUpdateVisuals(float deltaTime) {
    updateVisualState(deltaTime, _hitboxPixels, isFacingLeft());
}

void Player::onRenderVisual(sf::RenderTarget& target, const sf::Vector2f& position, float angleDegrees) {
    renderVisualState(target, position);
}
