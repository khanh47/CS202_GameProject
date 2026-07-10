#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Text.hpp>
#include <optional>
#include <memory>

#include "Animation/Animator.h"
#include "Physics/PhysicsBody.h"
#include "Physics/PhysicsWorld.h"

class GameObject {
public:
    virtual ~GameObject() = default;

    virtual void updateSimulation(const float &fixedDt);
    virtual void updateVisuals(float deltaTime);
    virtual void render(sf::RenderTarget &target);

    virtual void spawn(const PhysicsWorld &physicsWorld, sf::Vector2f spawnPixels, sf::Vector2f hitboxPixels);
    void destroy() { _pendingDestroy = true; }
    
    bool isPendingDestroy() { return _pendingDestroy; }

protected:
    virtual void onCreateBodyDef(b2BodyDef& def);
    virtual void onCreateShapeDef(b2ShapeDef& def);

    void createBody(const PhysicsWorld &physicsWorld, sf::Vector2f spawnPixels);
    void createHitbox(sf::Vector2f hitboxPixels);
    void updateSpriteLayout();


    std::shared_ptr<sf::Texture> _spritesheet;
    Animator _animator;

    std::optional<sf::Sprite> _sprite;
    sf::Vector2f _hitboxPixels{0.f, 0.f};

    std::shared_ptr<PhysicsBody> _body = nullptr;
    bool _pendingDestroy = false;
};
