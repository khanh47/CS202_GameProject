#include "Game/Objects/Entities/Player.h"
#include "Animation/AnimationClip.h"
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Vector2.hpp>

Player::Player() : GameObject(), Damageable(100) {
    if (_sprite.has_value()) {
        sf::FloatRect bounds = _sprite->getLocalBounds();
        _sprite->setOrigin({bounds.size.x / 2.f, bounds.size.y / 2.f});
    }
}

Player::Player(sf::Texture &texture) : GameObject(), Damageable(100) {
    _spritesheet.reset(&texture, [](sf::Texture*){});
    _sprite = sf::Sprite(*_spritesheet);

    AnimationClip idle = Animation::createClip(
            sf::IntRect(
                {0, 256 * 2},
                {256, 256}
            ), 
            sf::Vector2i(0, 0),
            8
        );
    
    _animator.addAnimation("idle", idle);
    _animator.play("idle");

    _sprite->setTextureRect(sf::IntRect({0, 0}, {37, 45}));

    sf::FloatRect bounds = _sprite->getLocalBounds();
    _sprite->setOrigin({bounds.size.x / 2.f, bounds.size.y / 2.f});
    // _sprite->setOrigin({100.0f, 100.0f});
}

Player::~Player() {
}

void Player::onInput(const sf::Event& event) {
    (void)event;
}

void Player::onCreateBodyDef(b2BodyDef& def) {
    def.type = b2_dynamicBody;
}
