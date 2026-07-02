#pragma once

#include <SFML/Graphics.hpp>

#include "Physics/PhysicsWorld.h"
#include "Game/Objects/GameObjectFactory.h"
#include "ResourceManager.h"

class GameWorld {
public:
    GameWorld();
    ~GameWorld() = default;

    void updateSimulation(const float &fixedDt);
    void updateVisuals(float deltaTime);
    void render(sf::RenderTarget &target);

    void test();

private:
    PhysicsWorld _physicsWorld;
    GameObjectFactory _objectFactory;
    std::vector<std::shared_ptr<GameObject>> _objects;

    // Grid system
    static constexpr float CELL_SIZE = 64.0f;
    int _gridWidth = 500;
    int _gridHeight = 60;
    std::vector<std::vector<std::shared_ptr<GameObject>>> _grid;

public:
    int getGridWidth() const { return _gridWidth; }
    int getGridHeight() const { return _gridHeight; }
    float getCellSize() const { return CELL_SIZE; }

private:
};