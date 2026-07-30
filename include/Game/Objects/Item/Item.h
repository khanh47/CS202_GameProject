#pragma once

#include <SFML/System.hpp>

#include "Game/Behaviours/Animatable.h"
#include "Game/Objects/GameObject.h"

class Item : public GameObject {
public:
    Item();
    Item(sf::Texture& texture);
    ~Item();

protected:
    std::unique_ptr<Animatable> animatable;

    void onUpdateVisuals(float deltaTime) override;
    void onRenderVisual(sf::RenderTarget& target, const sf::Vector2f& position, float angleDegrees) override;
};
