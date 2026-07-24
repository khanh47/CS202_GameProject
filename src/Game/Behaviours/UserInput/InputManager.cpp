#include "Game/Behaviours/UserInput/InputManager.h"
#include <SFML/Graphics.hpp>

using namespace std;

InputManager::InputManager() {
    BindKey(sf::Keyboard::Key::W, ActionType::MoveUp);
    BindKey(sf::Keyboard::Key::A, ActionType::MoveLeft);
    BindKey(sf::Keyboard::Key::S, ActionType::MoveDown);
    BindKey(sf::Keyboard::Key::D, ActionType::MoveRight);
    BindKey(sf::Keyboard::Key::Space, ActionType::Shoot);
}   

void InputManager::ProcessInput() {
    
}