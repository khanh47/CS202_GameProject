#include "Game/Objects/Player/Player.h"
#include "Animation/AnimationLibrary.h"
#include "Game/Objects/GameObject.h"
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Vector2.hpp>
#include <memory>

Player::Player() : GameObject(), Damageable(100) {
    if (_sprite.has_value()) {
        sf::FloatRect bounds = _sprite->getLocalBounds();
        _sprite->setOrigin({bounds.size.x / 2.f, bounds.size.y / 2.f});
    }
}

Player::Player(sf::Texture &texture) : Player(texture, "mario") {
}

Player::Player(sf::Texture &texture, const std::string& animationSetId) : GameObject(), Damageable(100) {
    _spritesheet.reset(&texture, [](sf::Texture*){});
    _sprite = sf::Sprite(*_spritesheet);

    std::shared_ptr<AnimationSet> animationSet = std::make_shared<AnimationSet>(
        AnimationLibrary::getInstance().getAnimationSet(animationSetId)
    );
    _animator = Animator(animationSet);

    _animator.play(animationSet->defaultClip);
    _animator.play("knockout");
    _sprite->setTextureRect(_animator.getCurrentTextureRect());

    sf::FloatRect bounds = _sprite->getLocalBounds();
    _sprite->setOrigin({bounds.size.x / 2.f, bounds.size.y / 2.f});
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
