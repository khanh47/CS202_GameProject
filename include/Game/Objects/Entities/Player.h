#pragma once

#include <box2d/box2d.h>
#include <SFML/System.hpp>

#include "Game/Behaviours/Controllable.h"
#include "Game/Behaviours/Damageable.h"
#include "Game/Objects/GameObject.h"
#include "Physics/PhysicsWorld.h"

class Player: public GameObject,
              public Damageable,
              public Controllable {
public:
    Player();
    Player(sf::Texture &texture);
    ~Player();

    void onInput(const sf::Event& event) override;

protected:
    void onCreateBodyDef(b2BodyDef& def) override;
};