#pragma once

#include "Game/Objects/Player/Player.h"
#include "Game/UserInput/InputManager.h"

#include <SFML/Window/Event.hpp>

class PlayerController: public InputManager {
public:
    enum class ControlScheme {
        Wasd,
        ArrowKeys
    };

    explicit PlayerController(Player& player, ControlScheme controlScheme = ControlScheme::Wasd);

    bool ActionStart(const sf::Event::KeyPressed& event) override;
    bool ActionEnd(const sf::Event::KeyReleased& event) override;

private:
    void bindControls(ControlScheme controlScheme);
    void applyPressAction(ActionType action);
    void applyReleaseAction(ActionType action);

    Player& _player;
};
