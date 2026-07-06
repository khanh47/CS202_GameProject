#pragma once

#include <SFML/Graphics.hpp>

#include "Physics/PhysicsWorld.h"
#include "Game/Objects/GameObjectFactory.h"
#include "ResourceManager.h"

class GameWorld {
public:
    GameWorld();
    ~GameWorld() = default;

    void handleInput(const sf::Event& event);
    void updateSimulation(const float &fixedDt);
    void updateVisuals(float deltaTime);
    void render(sf::RenderTarget &target);

    void test();

private:
    PhysicsWorld _physicsWorld;
    GameObjectFactory _objectFactory;
    std::vector<std::shared_ptr<GameObject>> _objects;
};