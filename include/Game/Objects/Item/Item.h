#pragma once

#include <SFML/System.hpp>

#include "Game/Objects/GameObject.h"

class Item : public GameObject {
public:
    Item();
    Item(sf::Texture& texture);
    ~Item();

protected:
    void onUpdateVisuals(float deltaTime) override;
    void onRenderVisual(sf::RenderTarget& target, const sf::Vector2f& position, float angleDegrees) override;
};
