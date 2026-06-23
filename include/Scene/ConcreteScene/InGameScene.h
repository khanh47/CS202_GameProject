#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "Scene/Scene.h"
#include "Physics/PhysicsWorld.h"
#include "Game/Objects/GameObject.h"
#include "Game/Objects/GameObjectFactory.h"



class InGameScene : public Scene {
public:
    explicit InGameScene(const std::string& name);
    ~InGameScene() override = default;

    void init() override;
    void onEnter() override;
    void onExit() override;
    void cleanup() override;

    void handleInput(const sf::Event& event) override;
    void updateSimulation(const float &fixedDt) override;
    void updateVisuals(float deltaTime) override;
    void render(sf::RenderTarget& target) override;

    std::string getName() const override { return _name; }
    bool isActive() const override { return _isActive; }

private:
    std::string _name;

    GameObjectFactory _objectFactory;
    std::shared_ptr<PhysicsWorld> _gameWorld = std::make_shared<PhysicsWorld>();
};
