#pragma once

#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>

#include "Game/Behaviours/Animatable.h"
#include "Game/Objects/GameObject.h"

/**
 * @brief Represents a bouncing fireball spawned by Fire Mario.
 * Uses Box2D dynamic physics for bouncing and manages its active/inactive pool lifecycle.
 */
class Fireball : public GameObject, public Animatable {
public:
    Fireball();
    Fireball(sf::Texture& texture);
    ~Fireball() override = default;

    void activate(sf::Vector2f spawnPos, bool facingRight);
    void deactivate();

    bool isActive() const { return _active; }
    float getDistanceTraveled() const { return _distanceTraveled; }

    void triggerBounce();
    void updateSimulation(const float& fixedDt) override;

protected:
    void onCreateBodyDef(b2BodyDef& def) override;
    void onCreateShapeDef(b2ShapeDef& def) override;
    void onUpdateVisuals(float deltaTime) override;
    void onRenderVisual(sf::RenderTarget& target, const sf::Vector2f& position, float angleDegrees) override;

private:
    bool _active = false;
    bool _facingRight = true;
    float _distanceTraveled = 0.0f;
    
    // Movement speeds in Box2D MKS matching original Super Mario Bros NES physics
    const float _moveSpeedMeters = 10.f;
    const float _bounceImpulseMeters = 7.5f;
};
