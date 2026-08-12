#pragma once

#include <box2d/box2d.h>
#include <SFML/System.hpp>

#include "Game/Objects/GameObject.h"
#include "Physics/PhysicsWorld.h"

class Block: public GameObject {
public:
    Block();
    Block(sf::Texture &texture);
    ~Block();

    void onContact(GameObject& other, const b2ContactData& contactData, b2ShapeId ownShape) override;
    virtual bool isRenderedByTileMap() const noexcept { return true; }

protected:
    bool isBumped(GameObject& other, const b2ContactData& contactData, b2ShapeId ownShape);
    void onCreateShapeDef(b2ShapeDef& def) override;
    void onUpdateVisuals(float deltaTime) override;
    void onRenderVisual(sf::RenderTarget& target, const sf::Vector2f& position, float angleDegrees) override;
};
