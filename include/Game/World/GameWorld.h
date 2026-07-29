#pragma once

#include <SFML/Graphics.hpp>
#include <memory>

#include "Physics/PhysicsWorld.h"
#include "Game/Objects/GameObjectFactory.h"
#include "Game/World/TileMap.h"
#include "Game/Objects/Item/FireballPool.h"

class PlayerController;

class GameWorld {
public:
    GameWorld();
    ~GameWorld();

    void handleInput(const sf::Event& event);
    void updateSimulation(const float &fixedDt);
    void updateVisuals(float deltaTime);
    void render(sf::RenderTarget &target);

    void test();
    void loadMap(const std::vector<std::vector<int>>& mapData);

    bool spawnFireball(sf::Vector2f spawnPos, bool facingRight);

private:
    PhysicsWorld _physicsWorld;
    GameObjectFactory _objectFactory;
    FireballPool _fireballPool;
    std::vector<std::shared_ptr<GameObject>> _objects;
    std::vector<std::unique_ptr<PlayerController>> _controllers;

    // TileMap system for batch vertex array rendering and tile culling
    TileMap _tileMap;

    // Grid system
    static constexpr float CELL_SIZE = 64.0f;
    int _gridWidth = 500;
    int _gridHeight = 60;
    int _loadedCols = 0;
    int _loadedRows = 0;
    std::vector<std::vector<std::shared_ptr<GameObject>>> _grid;

public:
    int getGridWidth() const { return _gridWidth; }
    int getGridHeight() const { return _gridHeight; }
    float getCellSize() const { return CELL_SIZE; }

    std::shared_ptr<GameObject> getPrimaryPlayer() const;
    sf::FloatRect getBounds() const;

private:
};
