#pragma once

#include <box2d/box2d.h>
#include <SFML/System.hpp>
#include <string>

#include "Game/Behaviours/Animatable.h"
#include "Game/Behaviours/Damageable.h"
#include "Game/Behaviours/Moveable.h"
#include "Game/Objects/GameObject.h"

class Player: public GameObject,
              public Animatable,
              public Damageable, 
              public Moveable {
public:
    Player();
    Player(sf::Texture &texture);
    Player(sf::Texture &texture, const std::string& animationSetId);
    ~Player();

protected:
    void updateSimulation(const float &fixedDt) override;
    void onCreateBodyDef(b2BodyDef& def) override;
    void onCreateShapeDef(b2ShapeDef& def) override;
    void onUpdateVisuals(float deltaTime) override;
    void onRenderVisual(sf::RenderTarget& target, const sf::Vector2f& position, float angleDegrees) override;

private:
    float _moveSpeed = 8.0f;
    float _jumpSpeed = 16.0f;
};
