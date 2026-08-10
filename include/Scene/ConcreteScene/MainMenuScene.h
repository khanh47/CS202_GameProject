#pragma once
#include "Scene/Scene.h"
#include "Button/ButtonMenu.h"

class MainMenuScene : public Scene {
public:
    MainMenuScene();
    ~MainMenuScene() override = default;

    void onEnter() override;
    void handleInput(const sf::Event& event) override;
    void render(sf::RenderTarget& target) override;

private:
    void _setupButtons();

    UI::ButtonMenu _buttonMenu;
    sf::Text _promptText;
    bool _showPrompt = true;
};
