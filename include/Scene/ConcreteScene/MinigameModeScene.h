#pragma once
#include "Scene/Scene.h"
#include "Button/ButtonMenu.h"
#include <string>

class MinigameModeScene : public Scene {
public:
    explicit MinigameModeScene(const std::string& mapPath);
    ~MinigameModeScene() override = default;

    void init() override;
    void onEnter() override;
    void onExit() override;
    void cleanup() override;

    void handleInput(const sf::Event& event) override;
    void updateVisuals(float deltaTime) override;
    void render(sf::RenderTarget& target) override;

    std::string getName() const override { return _name; }
    bool isActive() const override { return _isActive; }

private:
    void _setupButtons();

    std::string _mapPath;
    UI::ButtonMenu _buttonMenu;
    sf::Text _titleText;
    sf::Text _comingSoonText;
    bool _showComingSoon = false;
    std::string _name;
};
