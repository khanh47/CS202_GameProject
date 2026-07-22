#include "Game/Objects/Player/Player.h"
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Vector2.hpp>

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

void Player::onInput(const sf::Event& event) {
    (void)event;
}

void Player::onCreateBodyDef(b2BodyDef& def) {
    def.type = b2_dynamicBody;
}

void Player::onCreateShapeDef(b2ShapeDef& def) {
    def.density = 1.0f;
    def.material.friction = 0.0f;
}

void Player::onUpdateVisuals(float deltaTime) {
    updateVisualState(deltaTime, _hitboxPixels);
}

void Player::onRenderVisual(sf::RenderTarget& target, const sf::Vector2f& position, float angleDegrees) {
    renderVisualState(target, position, angleDegrees);
}
