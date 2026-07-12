#pragma once

#include <SFML/System.hpp>

#include "Game/Behaviours/Animatable.h"
#include "Game/Objects/GameObject.h"

class Item : public GameObject,
             public Animatable {
public:
    Item();
    Item(sf::Texture& texture);
    ~Item();
};
