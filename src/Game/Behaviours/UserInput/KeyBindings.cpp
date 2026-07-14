#include <iostream>
#include "Game/Behaviours/UserInput/Action.h"
#include "Game/Behaviours/UserInput/KeyBindings.h"

using namespace std;

void KeyBindings::BindKey(const sf::Keyboard::Key& key, const ActionType& action) {
    keyActionMap[key] = action;
}

void KeyBindings::UnbindKey(const sf::Keyboard::Key& key) {
    keyActionMap.erase(key);
}

ActionType KeyBindings::GetActionForKey(const sf::Keyboard::Key& key) const {
    auto it = keyActionMap.find(key);
    if (it != keyActionMap.end()) {
        return it->second;
    } else {
        cout << "No action bound to this key!" << endl;
        return ActionType::MoveLeft; 
    }
}