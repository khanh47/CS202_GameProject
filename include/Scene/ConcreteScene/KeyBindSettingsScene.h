#pragma once

#include "Scene/Scene.h"
#include "Button/ButtonMenu.h"
#include "Game/UserInput/Action.h"

#include <memory>
#include <optional>

class KeyBindSettingsScene : public Scene {
public:
    KeyBindSettingsScene();
    ~KeyBindSettingsScene() override = default;

    void init() override;
    void onEnter() override;
    void onExit() override;

    void handleInput(const sf::Event& event) override;
    void updateVisuals(float deltaTime) override;
    void render(sf::RenderTarget& target) override;

    void startRebinding(ActionType action, std::shared_ptr<UI::Button> button);

private:
    void updateKeybindButtonTexts();

    UI::ButtonMenu _menu;

    std::shared_ptr<UI::Button> _btnMoveLeft;
    std::shared_ptr<UI::Button> _btnMoveRight;
    std::shared_ptr<UI::Button> _btnJump;
    std::shared_ptr<UI::Button> _btnAttack;
    std::shared_ptr<UI::Button> _btnButton;
    std::shared_ptr<UI::Button> _btnBack;

    std::optional<ActionType> _rebindingAction;
    std::shared_ptr<UI::Button> _activeRebindingButton;
};
