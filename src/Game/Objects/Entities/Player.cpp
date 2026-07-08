#include "Game/Objects/Entities/Player.h"

Player::Player() : GameObject(), Damageable(100) {
    if (_sprite.has_value()) {
        sf::FloatRect bounds = _sprite.value().getLocalBounds();
        _sprite->setOrigin({bounds.size.x / 2.f, bounds.size.y / 2.f});
    }
}

Player::Player(sf::Texture &texture) : GameObject(), Damageable(100) {
    _sprite = sf::Sprite(texture);
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