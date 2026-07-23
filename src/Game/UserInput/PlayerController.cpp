#include "Game/UserInput/PlayerController.h"

PlayerController::PlayerController(Player& player, ControlScheme controlScheme)
    : _player(player) {
    bindControls(controlScheme);
}

bool PlayerController::ActionStart(const sf::Event::KeyPressed& event) {
    const auto action = GetActionForKey(event.code);
    if (!action.has_value()) {
        return false;
    }

    applyPressAction(*action);
    return true;
}

bool PlayerController::ActionEnd(const sf::Event::KeyReleased& event) {
    const auto action = GetActionForKey(event.code);
    if (!action.has_value()) {
        return false;
    }

    applyReleaseAction(*action);
    return true;
}

void PlayerController::bindControls(ControlScheme controlScheme) {
    switch (controlScheme) {
        case ControlScheme::Wasd:
            BindKey(sf::Keyboard::Key::A, ActionType::MoveLeft);
            BindKey(sf::Keyboard::Key::D, ActionType::MoveRight);
            BindKey(sf::Keyboard::Key::W, ActionType::MoveUp);
            BindKey(sf::Keyboard::Key::S, ActionType::MoveDown);
            break;
        case ControlScheme::ArrowKeys:
            BindKey(sf::Keyboard::Key::Left, ActionType::MoveLeft);
            BindKey(sf::Keyboard::Key::Right, ActionType::MoveRight);
            BindKey(sf::Keyboard::Key::Up, ActionType::MoveUp);
            BindKey(sf::Keyboard::Key::Down, ActionType::MoveDown);
            break;
    }
}

void PlayerController::applyPressAction(ActionType action) {
    switch (action) {
        case ActionType::MoveLeft:
            _player.startMoveLeft();
            break;
        case ActionType::MoveRight:
            _player.startMoveRight();
            break;
        case ActionType::MoveUp:
            _player.startJump();
            break;
        case ActionType::MoveDown:
        case ActionType::Accelerate:
        case ActionType::Decelerate:
            break;
    }
}

void PlayerController::applyReleaseAction(ActionType action) {
    switch (action) {
        case ActionType::MoveLeft:
            _player.stopMoveLeft();
            break;
        case ActionType::MoveRight:
            _player.stopMoveRight();
            break;
        case ActionType::MoveUp:
            _player.stopJump();
        case ActionType::MoveDown:
        case ActionType::Accelerate:
        case ActionType::Decelerate:
            break;
    }
}
