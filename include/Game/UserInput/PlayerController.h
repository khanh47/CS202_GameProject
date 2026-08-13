#pragma once

#include "Game/Objects/Player/Player.h"
#include "Game/UserInput/InputManager.h"
#include "Game/UserInput/IPlayerController.h"

#include <SFML/Window/Event.hpp>

class GameWorld;

class PlayerController: public InputManager, public IPlayerController {
public:
    enum class ControlScheme {
        Wasd,
        ArrowKeys
    };

    explicit PlayerController(Player& player, GameWorld& gameWorld, ControlScheme controlScheme = ControlScheme::Wasd);

    bool handleEvent(const sf::Event& event) override {
        return InputManager::handleEvent(event);
    }
    void fixedUpdate(float fixedDt) override { (void)fixedDt; }
    void syncState() override { syncStateWithKeyboard(); }

    bool ActionStart(const sf::Event::KeyPressed& event) override;
    bool ActionEnd(const sf::Event::KeyReleased& event) override;

    void syncStateWithKeyboard();
    bool isPlayerPendingDestroy() const override { return _player.isPendingDestroy(); }
    void refreshBindings();

private:
    void bindControls(ControlScheme controlScheme);
    void applyPressAction(ActionType action);
    void applyReleaseAction(ActionType action);

    Player& _player;
    GameWorld& _gameWorld;
    ControlScheme _controlScheme;
};
