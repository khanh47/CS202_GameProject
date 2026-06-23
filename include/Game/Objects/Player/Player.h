#pragma once

#include <box2d/box2d.h>
#include <SFML/System.hpp>

#include "Game/Behaviours/Damageable.h"
#include "Game/Objects/GameObject.h"
#include "Physics/PhysicsWorld.h"

class Player: public GameObject,
              public Damageable {
public:
    Player();
    Player(sf::Texture &texture);
    ~Player();

    void spawn(PhysicsWorld* physicsWorld, sf::Vector2f spawnPixels, sf::Vector2f hitboxPixels) override;
private:
    void createBody(PhysicsWorld* physicsWorld, sf::Vector2f spawnPixels);
    void createHitbox(sf::Vector2f hitboxPixels);
};