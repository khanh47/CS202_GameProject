#pragma once

#include "Scene/Scene.h"
#include "Button/ButtonMenu.h"
#include "Game/UserInput/Action.h"

#include <memory>
#include <optional>
#include <string>

class KeyBindSettingsScene : public Scene {
public:
    KeyBindSettingsScene();
    ~KeyBindSettingsScene() override = default;

    void init() override;
    void onEnter() override;
    void onExit() override;
    void cleanup() override;

    void handleInput(const sf::Event& event) override;
    void updateVisuals(float deltaTime) override;
    void render(sf::RenderTarget& target) override;

    std::string getName() const override { return _name; }
    bool isActive() const override { return _isActive; }

    void startRebinding(ActionType action, std::shared_ptr<UI::Button> button);

private:
    void updateKeybindButtonTexts();

    std::string _name;
    UI::ButtonMenu _menu;

    std::shared_ptr<UI::Button> _btnMoveLeft;
    std::shared_ptr<UI::Button> _btnMoveRight;
    std::shared_ptr<UI::Button> _btnJump;
    std::shared_ptr<UI::Button> _btnAttack;
    std::shared_ptr<UI::Button> _btnBack;

    std::optional<ActionType> _rebindingAction;
    std::shared_ptr<UI::Button> _activeRebindingButton;
};
