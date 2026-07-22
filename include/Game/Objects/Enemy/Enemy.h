#pragma once

#include <box2d/box2d.h>
#include <SFML/System.hpp>

#include "Game/Behaviours/Animatable.h"
#include "Game/Behaviours/Damageable.h"
#include "Game/Objects/GameObject.h"
#include "Physics/PhysicsWorld.h"

class Enemy : public GameObject,
              public Animatable,
              public Damageable {
public:
    Enemy();
    Enemy(sf::Texture& texture);
    ~Enemy();

protected:
    void onCreateBodyDef(b2BodyDef& def) override;
    void onUpdateVisuals(float deltaTime) override;
    void onRenderVisual(sf::RenderTarget& target, const sf::Vector2f& position, float angleDegrees) override;
};
