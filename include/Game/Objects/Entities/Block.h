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

protected:
    void onCreateShapeDef(b2ShapeDef& def) override;
};