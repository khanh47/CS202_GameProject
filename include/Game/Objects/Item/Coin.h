#pragma once

#include <box2d/box2d.h>
#include "Game/Behaviours/Animatable.h"
#include "Game/Behaviours/Damageable.h"
#include "Game/Behaviours/Moveable.h"
#include "Game/Objects/GameObject.h"
#include "Physics/PhysicsWorld.h"

class Coin : public GameObject,
               public Animatable,
               public Damageable,
               public Moveable {
public:
    Coin();
    Coin(sf::Texture& texture);
    ~Coin();

protected:
    void onCreateBodyDef(b2BodyDef& def) override;
    void onCreateShapeDef(b2ShapeDef& def) override;
    void onUpdateVisuals(float deltaTime) override;
    void onRenderVisual(sf::RenderTarget& target,
                        const sf::Vector2f& position,
                        float angleDegrees) override;
    void updateSimulation(const float& fixedDt) override;
};