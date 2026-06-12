#pragma once
#include "Scene.h"
#include <string>

class SettingsScene : public Scene {
public:
    SettingsScene();
    ~SettingsScene() override = default;

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
    std::string _name;
};
