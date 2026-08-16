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

    void setBreakable(bool breakable) noexcept { _breakable = breakable; }
    bool isBreakable() const noexcept { return _breakable; }
    void setBreakEffectTexture(
        const sf::Texture* texture,
        sf::IntRect textureRect = {}
    ) noexcept {
        _breakTexture = texture;
        _breakTextureRect = textureRect;
    }

    void onContact(GameObject& other, const b2ContactData& contactData, b2ShapeId ownShape) override;
    virtual bool isRenderedByTileMap() const noexcept { return true; }

protected:
    bool tryBreakOnContact(
        GameObject& other,
        const b2ContactData& contactData,
        b2ShapeId ownShape
    );
    bool isBumped(GameObject& other, const b2ContactData& contactData, b2ShapeId ownShape);
    void onCreateShapeDef(b2ShapeDef& def) override;
    void onUpdateVisuals(float deltaTime) override;
    void onRenderVisual(sf::RenderTarget& target, const sf::Vector2f& position, float angleDegrees) override;

private:
    bool _breakable = false;
    const sf::Texture* _breakTexture = nullptr;
    sf::IntRect _breakTextureRect{};
};
