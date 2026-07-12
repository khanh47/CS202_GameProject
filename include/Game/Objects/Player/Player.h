#pragma once

#include <box2d/box2d.h>
#include <SFML/System.hpp>
#include <string>

#include "Game/Behaviours/Controllable.h"
#include "Game/Behaviours/Damageable.h"
#include "Game/Objects/GameObject.h"

class Player: public GameObject,
              public Damageable,
              public Controllable {
public:
    Player();
    Player(sf::Texture &texture);
    Player(sf::Texture &texture, const std::string& animationSetId);
    ~Player();

    void onInput(const sf::Event& event) override;

protected:
    void onCreateBodyDef(b2BodyDef& def) override;
    void onCreateShapeDef(b2ShapeDef& def) override;
};
