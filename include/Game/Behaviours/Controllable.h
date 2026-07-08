#pragma once

#include <SFML/Graphics.hpp>

class Controllable {
public:
    virtual ~Controllable() = default;
    virtual void onInput(const sf::Event& event) = 0;
};
