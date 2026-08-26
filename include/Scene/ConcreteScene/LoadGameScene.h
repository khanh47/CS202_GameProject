#pragma once

#include "Button/ButtonMenu.h"
#include "Scene/Scene.h"

class LoadGameScene : public Scene {
public:
    LoadGameScene();
    ~LoadGameScene() override = default;

    void onEnter() override;
    void handleInput(const sf::Event& event) override;
    void render(sf::RenderTarget& target) override;

private:
    void setupButtons();
    void loadFromSlot(int index);
    void setStatus(const std::string& status, sf::Color color = sf::Color::White);

    UI::ButtonMenu _buttonMenu;
    sf::Text _titleText;
    sf::Text _statusText;
};
