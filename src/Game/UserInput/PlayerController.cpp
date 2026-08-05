#include "Game/UserInput/PlayerController.h"
#include "Game/World/GameWorld.h"
#include "Game/GameSettings.h"

PlayerController::PlayerController(Player& player, GameWorld& gameWorld, ControlScheme controlScheme)
    : _player(player), _gameWorld(gameWorld), _controlScheme(controlScheme) {
    bindControls(controlScheme);
}

bool PlayerController::ActionStart(const sf::Event::KeyPressed& event) {
    refreshBindings();
    const auto action = GetActionForKey(event.code);
    if (!action.has_value()) {
        return false;
    }

    applyPressAction(*action);
    return true;
}

bool PlayerController::ActionEnd(const sf::Event::KeyReleased& event) {
    refreshBindings();
    const auto action = GetActionForKey(event.code);
    if (!action.has_value()) {
        return false;
    }

    applyReleaseAction(*action);
    return true;
}

void PlayerController::bindControls(ControlScheme controlScheme) {
    _controlScheme = controlScheme;
    refreshBindings();
}

void PlayerController::refreshBindings() {
    ClearBindings();
    if (_controlScheme == ControlScheme::Wasd) {
        const auto& settings = GameSettings::getInstance();
        BindKey(settings.keyMoveLeft, ActionType::MoveLeft);
        BindKey(settings.keyMoveRight, ActionType::MoveRight);
        BindKey(settings.keyJump, ActionType::MoveUp);
        BindKey(sf::Keyboard::Key::S, ActionType::MoveDown);
        BindKey(settings.keyAttack, ActionType::Attack);
    } else {
        BindKey(sf::Keyboard::Key::Left, ActionType::MoveLeft);
        BindKey(sf::Keyboard::Key::Right, ActionType::MoveRight);
        BindKey(sf::Keyboard::Key::Up, ActionType::MoveUp);
        BindKey(sf::Keyboard::Key::Down, ActionType::MoveDown);
        BindKey(sf::Keyboard::Key::M, ActionType::Attack);
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
    refreshBindings();
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
