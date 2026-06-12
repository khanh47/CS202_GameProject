#pragma once
#include "Scene/Scene.h"
#include "Button/ButtonMenu.h"
#include <string>

class MainMenuScene : public Scene {
public:
    MainMenuScene();
    ~MainMenuScene() override = default;

    void init() override;
    void onEnter() override;
    void onExit() override;
    void cleanup() override;

    void handleInput(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderTarget& target) override;

    std::string getName() const override { return _name; }
    bool isActive() const override { return _isActive; }

private:
    void _setupButtons();

    UI::ButtonMenu _buttonMenu;
    sf::Text _promptText;
    bool _showPrompt = true;
    std::string _name;
};
