#include <SFML/Graphics.hpp>
#include <iostream>
#include <unordered_map>
#include <map>
#include <string>
#include "Action.h"

#pragma once

using namespace std;

class KeyBindings {
private:
    unordered_map <sf::Keyboard::Key, ActionType> keyActionMap;
public:
    KeyBindings() = default;
    ~KeyBindings() = default;

    void BindKey(const sf::Keyboard::Key& key, const ActionType& action);
    void UnbindKey(const sf::Keyboard::Key& key);
    ActionType GetActionForKey(const sf::Keyboard::Key& key) const;
}; 