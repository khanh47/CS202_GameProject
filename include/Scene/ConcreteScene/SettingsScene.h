#pragma once

#include "Scene/Scene.h"
#include "Button/ButtonMenu.h"

class SettingsScene : public Scene {
public:
    SettingsScene();
    ~SettingsScene() override = default;

    void init() override;
    void handleInput(const sf::Event& event) override;
    void updateVisuals(float deltaTime) override;
    void render(sf::RenderTarget& target) override;

private:
    UI::ButtonMenu _menu;
};
