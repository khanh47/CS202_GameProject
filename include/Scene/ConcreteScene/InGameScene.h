#pragma once
#include "Scene/Scene.h"
#include <string>

class InGameScene : public Scene {
public:
    explicit InGameScene(const std::string& name);
    ~InGameScene() override = default;

    void init() override;
    void onEnter() override;
    void onExit() override;
    void cleanup() override;

    void handleInput(const sf::Event& event) override;
    void updateSimulation(const float &fixedDt, const int &subSteps) override;
    void updateVisuals(float deltaTime) override;
    void render(sf::RenderTarget& target) override;

    std::string getName() const override { return _name; }
    bool isActive() const override { return _isActive; }

private:
    std::string _name;
};
