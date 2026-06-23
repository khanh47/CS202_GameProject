#pragma once

#include <box2d/box2d.h>
#include <SFML/System.hpp>

#include "Game/Objects/GameObject.h"
#include "Physics/PhysicsWorld.h"

class Block: public GameObject {
public:
    Block();
    Block(sf::Texture &texture);
    ~Block();

    void spawn(PhysicsWorld* physicsWorld, sf::Vector2f spawnPixels, sf::Vector2f hitboxPixels) override;
private:
    void createBody(PhysicsWorld* physicsWorld, sf::Vector2f spawnPixels);
    void createHitbox(sf::Vector2f hitboxPixels);
};