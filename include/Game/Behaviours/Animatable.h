#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <optional>
#include <string>

#include "Animation/Animator.h"

class Animatable {
public:
    Animatable() = default;
    Animatable(sf::Texture &texture);
    Animatable(sf::Texture &texture, const std::string& animationSetId);
    virtual ~Animatable() = default;

protected:
    void configureVisuals(sf::Texture& texture);
    void configureVisuals(sf::Texture& texture, const std::string& animationSetId);

    void updateVisualState(float deltaTime, const sf::Vector2f& hitboxPixels);
    void renderVisualState(sf::RenderTarget& target, const sf::Vector2f& position, float angleDegrees) const;

    void playAnimation(const std::string& name);
    void stopAnimation();

    bool hasSprite() const;

private:
    void bindTexture(sf::Texture& texture);
    void syncSpriteLayout(const sf::Vector2f& hitboxPixels);

    std::shared_ptr<sf::Texture> _spritesheet;
    Animator _animator;
    std::optional<sf::Sprite> _sprite;
};
