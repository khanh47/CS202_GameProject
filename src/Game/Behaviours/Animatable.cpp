#include "Game/Behaviours/Animatable.h"
#include  "iostream"

#include "Animation/AnimationLibrary.h"
#include "Physics/PhysicsUnits.h"

#include <SFML/System/Angle.hpp>
#include <memory>

Animatable::Animatable(sf::Texture &texture) {
    configureVisuals(texture);
}

Animatable::Animatable(sf::Texture &texture, const std::string& animationSetId) {
    configureVisuals(texture, animationSetId);
}

void Animatable::configureVisuals(sf::Texture& texture) {
    bindTexture(texture);
    _animator = Animator();
}

void Animatable::setTextureRect(const sf::IntRect& rect) {
    if (_sprite) {
        _sprite->setTextureRect(rect);
    }
}

void Animatable::configureVisuals(sf::Texture& texture, const std::string& animationSetId) {
    bindTexture(texture);
    try {
        std::shared_ptr<AnimationSet> animationSet = std::make_shared<AnimationSet>(
            AnimationLibrary::getInstance().getAnimationSet(animationSetId)
        );
        _animator = Animator(animationSet);
        _animator.play(animationSet->defaultClip);

        if (_sprite.has_value() && _animator.hasActiveAnimation()) {
            _sprite->setTextureRect(_animator.getCurrentTextureRect());
        } else {
            std::cout << "Animation FAILED: no active animation for " << animationSetId << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "Animation EXCEPTION: " << e.what() << std::endl;
        _animator = Animator();
    }
}

void Animatable::updateVisualState(float deltaTime, const sf::Vector2f& hitboxPixels, bool facingLeft) {
    const bool frameChanged = _animator.update(deltaTime);

    if (_sprite && _animator.hasActiveAnimation()) {
        const sf::IntRect currentRect = _animator.getCurrentTextureRect();
        if (frameChanged || _sprite->getTextureRect() != currentRect) {
            _sprite->setTextureRect(currentRect);
        }
    }

    syncSpriteLayout(hitboxPixels, facingLeft);
}

void Animatable::renderVisualState(sf::RenderTarget& target, const sf::Vector2f& position, float angleDegrees) const {
    if (!_sprite.has_value()) {
        return;
    }

    sf::Sprite sprite = *_sprite;
    sprite.setPosition(position);
    sprite.setRotation(sf::degrees(angleDegrees));
    target.draw(sprite);
}

void Animatable::playAnimation(const std::string& name) {
    if (_animator.getActiveAnimationName() == name) return;
    _animator.play(name);

    if (_sprite && _animator.hasActiveAnimation()) {
        _sprite->setTextureRect(_animator.getCurrentTextureRect());
    }
}

void Animatable::stopAnimation() {
    _animator.stop();
}

std::string Animatable::getActiveAnimationName() const {
    return _animator.getActiveAnimationName();
}

bool Animatable::hasSprite() const {
    return _sprite.has_value();
}

void Animatable::bindTexture(sf::Texture& texture) {
    _spritesheet.reset(&texture, [](sf::Texture*) {});
    _sprite = sf::Sprite(*_spritesheet);
}

void Animatable::syncSpriteLayout(const sf::Vector2f& hitboxPixels, bool facingLeft) {
    if (!_sprite || hitboxPixels.x <= 0.f || hitboxPixels.y <= 0.f) {
        return;
    }

    const sf::Vector2i frameSize = _sprite->getTextureRect().size;
    if (frameSize.x <= 0 || frameSize.y <= 0) {
        return;
    }


    _sprite->setOrigin({frameSize.x / 2.f, frameSize.y / 2.f});

    float xOrientation = 1.0f;
    if(facingLeft) xOrientation = -1.0f;
    _sprite->setScale({
        xOrientation * hitboxPixels.x / static_cast<float>(frameSize.x),
        hitboxPixels.y / static_cast<float>(frameSize.y)
    });
}
