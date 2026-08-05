#pragma once

#include <SFML/Graphics.hpp>
#include <array>
#include <memory>
#include <string>

#include "Game/Objects/GameObjectFactory.h"
#include "Game/Objects/Item/FireballPool.h"
#include "Game/World/WorldInteractionSystem.h"
#include "Game/World/WorldMap.h"
#include "Game/World/WorldObjectStore.h"
#include "Game/World/WorldRenderer.h"
#include "Game/ScoreManager.h"
#include "Physics/PhysicsWorld.h"

class GameWorld {
public:
    GameWorld();
    ~GameWorld();

    void handleInput(const sf::Event& event);
    void updateSimulation(const float& fixedDt);
    void updateVisuals(float deltaTime);
    void render(sf::RenderTarget& target);
    void handleContacts(b2ContactEvents contactEvents);
    void handleSensors(b2SensorEvents sensorEvents);

    void loadLevel(const std::string& levelPath);
    void loadMap(const LevelData& levelData);

    bool spawnFireball(sf::Vector2f spawnPos, bool facingRight, int playerIndex);
    void freeze(float durationSeconds);
    bool isFrozen() const { return _freezeTimer > 0.0f; }
    void syncPlayerControllers();

    int getGridWidth() const { return _worldMap.getGridWidth(); }
    int getGridHeight() const { return _worldMap.getGridHeight(); }
    float getCellSize() const { return _worldMap.getCellSize(); }
    std::shared_ptr<GameObject> getPrimaryPlayer() const;
    sf::FloatRect getBounds() const;

    // Add getter & setter for ScoreManager
    void setScoreManager(ScoreManager* scoreManager) { _scoreManager = scoreManager; }
    ScoreManager* getScoreManager() const { return _scoreManager; }

private:
    ScoreManager* _scoreManager = nullptr;
    float _freezeTimer = 0.0f;
    PhysicsWorld _physicsWorld;
    GameObjectFactory _objectFactory;
    std::array<FireballPool, 2> _fireballPools;
    WorldObjectStore _objectStore;
    WorldMap _worldMap;
    WorldInteractionSystem _interactionSystem;
    WorldRenderer _renderer;
};
