#pragma once

#include <box2d/box2d.h>
#include <SFML/System.hpp>

#include "Game/Behaviours/Animatable.h"
#include "Game/Objects/GameObject.h"
#include "Physics/PhysicsWorld.h"

class Block: public GameObject,
             public Animatable {
public:
    Block();
    Block(sf::Texture &texture);
    ~Block();

protected:
    void onCreateShapeDef(b2ShapeDef& def) override;
    void onUpdateVisuals(float deltaTime) override;
    void onRenderVisual(sf::RenderTarget& target, const sf::Vector2f& position, float angleDegrees) override;
};
