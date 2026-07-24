#include <SFML/Graphics.hpp>
#include "KeyBindings.h"
#include "Action.h"

#pragma once

using namespace std;

class InputManager : public KeyBindings, public Action {
private:

public:
    InputManager();
    ~InputManager() = default;
   
    void ProcessInput();

    virtual bool ActionStart(const sf::Event& event) = 0;
    virtual bool ActionEnd(const sf::Event& event) = 0;
};