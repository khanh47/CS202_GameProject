#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Text.hpp>
#include <memory>

#include "Physics/PhysicsBody.h"
#include "Physics/PhysicsWorld.h"

class GameObject {
public:
    GameObject();
    virtual ~GameObject() = default;

    virtual void updateSimulation(const float &fixedDt);
    virtual void updateVisuals(float deltaTime);
    virtual void render(sf::RenderTarget &target);

    virtual void spawn(const PhysicsWorld &physicsWorld, sf::Vector2f spawnPixels, sf::Vector2f hitboxPixels, bool hasFeet = false);
    void destroy() { _pendingDestroy = true; }
    
    bool isPendingDestroy() { return _pendingDestroy; }
    sf::Vector2f getPosition() const;
    virtual sf::Vector2f getVelocity() const;

protected:
    virtual void onCreateBodyDef(b2BodyDef& def);
    virtual void onCreateShapeDef(b2ShapeDef& def);
    virtual void onUpdateVisuals(float deltaTime);
    virtual void onRenderVisual(sf::RenderTarget& target, const sf::Vector2f& position, float angleDegrees);
    virtual void onHitboxRecreated();

    
    bool hasValidBody() const;
    sf::Vector2f getBodyPositionPixels() const;
    float getBodyAngleDegrees() const;
    void updateHitboxSize(sf::Vector2f newHitboxPixels);

    sf::Vector2f _hitboxPixels{0.f, 0.f};
    sf::Vector2f _baseHitboxPixels{0.f, 0.f};

    std::shared_ptr<PhysicsBody> _body = nullptr;
    bool _pendingDestroy = false;

private:
    void createBody(const PhysicsWorld &physicsWorld, sf::Vector2f spawnPixels);
    void createHitbox(sf::Vector2f hitboxPixels);
    void createFeet(sf::Vector2f hitboxPixels);
    void drawFallbackRect(sf::RenderTarget& target) const; // debugging

    bool _hasFeet = false;
    b2ShapeId _feetShapeId = b2_nullShapeId;
};
