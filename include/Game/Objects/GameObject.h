#pragma once

#include <SFML/Graphics.hpp>
#include <optional>
#include <memory>

#include "Physics/PhysicsBody.h"
#include "Physics/PhysicsWorld.h"

class GameObject {
public:
    virtual ~GameObject() = default;

    virtual void updateSimulation(const float &fixedDt);
    virtual void updateVisuals(float deltaTime);
    virtual void render(sf::RenderTarget &target);

    virtual void spawn(const PhysicsWorld &physicsWorld, sf::Vector2f spawnPixels, sf::Vector2f hitboxPixels) = 0;
    void destroy() { _pendingDestroy = true; }
    
    bool isPendingDestroy() { return _pendingDestroy; }

protected:
    std::optional<sf::Sprite> _sprite;
    std::shared_ptr<PhysicsBody> _body = nullptr;
    bool _pendingDestroy = false;
};