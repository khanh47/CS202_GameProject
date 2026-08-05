#include "Game/UserInput/PlayerController.h"
#include "Game/World/GameWorld.h"

PlayerController::PlayerController(Player& player, GameWorld& gameWorld, ControlScheme controlScheme)
    : _player(player), _gameWorld(gameWorld) {
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
            BindKey(sf::Keyboard::Key::X, ActionType::Attack);
            break;
        case ControlScheme::ArrowKeys:
            BindKey(sf::Keyboard::Key::Left, ActionType::MoveLeft);
            BindKey(sf::Keyboard::Key::Right, ActionType::MoveRight);
            BindKey(sf::Keyboard::Key::Up, ActionType::MoveUp);
            BindKey(sf::Keyboard::Key::Down, ActionType::MoveDown);
            BindKey(sf::Keyboard::Key::Enter, ActionType::Attack);
            break;
    }
}

void PlayerController::applyPressAction(ActionType action) {
    switch (action) {
        case ActionType::MoveLeft:
            _player.stopMoveRight();
            _player.startMoveLeft();
            break;
        case ActionType::MoveRight:
            _player.stopMoveLeft();
            _player.startMoveRight();
            break;
        case ActionType::MoveUp:
            _player.startJump();
            break;
        case ActionType::Attack:
            _player.attack(_gameWorld);
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

void PlayerController::syncStateWithKeyboard() {
    bool moveLeftPressed = false;
    bool moveRightPressed = false;

    for (const auto& [key, action] : getKeyActionMap()) {
        if (sf::Keyboard::isKeyPressed(key)) {
            if (action == ActionType::MoveLeft) {
                moveLeftPressed = true;
            } else if (action == ActionType::MoveRight) {
                moveRightPressed = true;
            }
        }
    }

    if (moveLeftPressed && !moveRightPressed) {
        _player.stopMoveRight();
        _player.startMoveLeft();
    } else if (moveRightPressed && !moveLeftPressed) {
        _player.stopMoveLeft();
        _player.startMoveRight();
    } else {
        _player.stopMoveLeft();
        _player.stopMoveRight();
    }
}
