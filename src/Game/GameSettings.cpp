#include "Game/GameSettings.h"

GameSettings& GameSettings::getInstance() {
    static GameSettings instance;
    return instance;
}

sf::Keyboard::Key GameSettings::getKeyForAction(ActionType action) const {
    switch (action) {
        case ActionType::MoveLeft:
            return keyMoveLeft;
        case ActionType::MoveRight:
            return keyMoveRight;
        case ActionType::MoveUp:
            return keyJump;
        case ActionType::Attack:
            return keyAttack;
        default:
            return sf::Keyboard::Key::Unknown;
    }
}

void GameSettings::setKeyForAction(ActionType action, sf::Keyboard::Key key) {
    switch (action) {
        case ActionType::MoveLeft:
            keyMoveLeft = key;
            break;
        case ActionType::MoveRight:
            keyMoveRight = key;
            break;
        case ActionType::MoveUp:
            keyJump = key;
            break;
        case ActionType::Attack:
            keyAttack = key;
            break;
        default:
            break;
    }
}

std::string GameSettings::keyToString(sf::Keyboard::Key key) {
    if (key >= sf::Keyboard::Key::A && key <= sf::Keyboard::Key::Z) {
        char c = static_cast<char>('A' + (static_cast<int>(key) - static_cast<int>(sf::Keyboard::Key::A)));
        return std::string(1, c);
    }
    if (key >= sf::Keyboard::Key::Num0 && key <= sf::Keyboard::Key::Num9) {
        char c = static_cast<char>('0' + (static_cast<int>(key) - static_cast<int>(sf::Keyboard::Key::Num0)));
        return std::string(1, c);
    }
    if (key >= sf::Keyboard::Key::Numpad0 && key <= sf::Keyboard::Key::Numpad9) {
        char c = static_cast<char>('0' + (static_cast<int>(key) - static_cast<int>(sf::Keyboard::Key::Numpad0)));
        return "Num " + std::string(1, c);
    }

    switch (key) {
        case sf::Keyboard::Key::Left:      return "Left";
        case sf::Keyboard::Key::Right:     return "Right";
        case sf::Keyboard::Key::Up:        return "Up";
        case sf::Keyboard::Key::Down:      return "Down";
        case sf::Keyboard::Key::Space:     return "Space";
        case sf::Keyboard::Key::Enter:     return "Enter";
        case sf::Keyboard::Key::Escape:    return "Escape";
        case sf::Keyboard::Key::LShift:    return "LShift";
        case sf::Keyboard::Key::RShift:    return "RShift";
        case sf::Keyboard::Key::LControl:  return "LCtrl";
        case sf::Keyboard::Key::RControl:  return "RCtrl";
        case sf::Keyboard::Key::LAlt:      return "LAlt";
        case sf::Keyboard::Key::RAlt:      return "RAlt";
        case sf::Keyboard::Key::Tab:       return "Tab";
        case sf::Keyboard::Key::Backspace: return "Backspace";
        default:
            return "Key " + std::to_string(static_cast<int>(key));
    }
}
