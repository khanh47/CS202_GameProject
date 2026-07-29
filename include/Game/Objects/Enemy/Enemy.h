#pragma once

#include <box2d/box2d.h>
#include <SFML/System.hpp>
#include <memory>

#include "Game/Behaviours/Animatable.h"
#include "Game/Behaviours/Damageable.h"
#include "Game/Objects/GameObject.h"

class Enemy : public GameObject {
public:
    Enemy();
    Enemy(sf::Texture& texture);
    Enemy(sf::Texture &texture, const std::string& animationSetId);
    ~Enemy();

protected:
    std::unique_ptr<Animatable> animatable;
    std::unique_ptr<Damageable> damageable;

    virtual void onCreateBodyDef(b2BodyDef& def) override;
    virtual void onCreateShapeDef(b2ShapeDef& def) override;
    virtual void onUpdateVisuals(float deltaTime) override;
    virtual void onRenderVisual(sf::RenderTarget& target, const sf::Vector2f& position, float angleDegrees) override;
    virtual void updateSimulation(const float &fixedDt) override;
    virtual void onContact();
private:
    float _moveSpeed = 2.0f;
};
