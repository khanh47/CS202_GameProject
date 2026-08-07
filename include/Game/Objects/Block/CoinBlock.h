#pragma once

#include <box2d/box2d.h>
#include <SFML/System.hpp>

#include "Game/Objects/GameObject.h"
#include "Physics/PhysicsWorld.h"

class CoinBlock: public GameObject {
public:
    CoinBlock();
    CoinBlock(sf::Texture &texture);
    ~CoinBlock();

protected:
    void onCreateShapeDef(b2ShapeDef& def) override;
    void onUpdateVisuals(float deltaTime) override;
    void onRenderVisual(sf::RenderTarget& target, const sf::Vector2f& position, float angleDegrees) override;

private:
    int capacity = 10; // Number of coins the block can give
};
