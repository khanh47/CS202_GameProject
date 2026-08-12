#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/System/Vector2.hpp>
#include <array>
#include <cstddef>
#include <memory>
#include <string>

#include "Game/Objects/GameObjectFactory.h"
#include "Game/Objects/Projectile/FireballPool.h"
#include "Game/World/WorldInteractionSystem.h"
#include "Game/World/WorldMap.h"
#include "Game/World/WorldObjectStore.h"
#include "Game/World/WorldRenderer.h"
#include "Game/ScoreManager.h"
#include "Physics/PhysicsWorld.h"

class GameWorld {
public:
    explicit GameWorld(
        int gridWidth = WorldMap::defaultGridWidth,
        int gridHeight = WorldMap::defaultGridHeight,
        float cellSize = WorldMap::defaultCellSize
    );
    ~GameWorld();

    void handleInput(const sf::Event& event);
    void updateSimulation(const float& fixedDt);
    void updateVisuals(float deltaTime);
    void render(sf::RenderTarget& target);
    void handleContacts(b2ContactEvents contactEvents);
    void handleSensors(b2SensorEvents sensorEvents);

    void loadLevel(const std::string& levelPath);
    void loadMap(const LevelData& levelData);
    void saveCheckpoint(sf::Vector2f position);
    void respawnPlayer();
    void reachFlagpole(sf::Vector2f position);

    bool spawnFireball(sf::Vector2f spawnPos, bool facingRight, int playerIndex);
    bool spawnKoopaShell(sf::Vector2f spawnPos, bool facingRight);
    bool spawnKoopa(sf::Vector2f spawnPos, bool facingRight);
    std::shared_ptr<GameObject> spawnItem(const std::string& itemTypeKey, sf::Vector2f position, sf::Vector2f size = {54.0f, 54.0f});
    void freeze(float durationSeconds);
    bool isFrozen() const { return _freezeTimer > 0.0f; }
    void syncPlayerControllers();
    void playVictoryAnimation();

    int getGridWidth() const { return _worldMap.getGridWidth(); }
    int getGridHeight() const { return _worldMap.getGridHeight(); }
    float getCellSize() const { return _worldMap.getCellSize(); }
    std::shared_ptr<GameObject> getPrimaryPlayer() const;
    bool hasLivingPlayers() const;
    bool hasWon() const { return _levelCleared; }
    sf::FloatRect getBounds() const;
    const std::vector<std::shared_ptr<GameObject>>& objects() const { return _objectStore.objects(); }

    // Add getter & setter for ScoreManager
    void setScoreManager(ScoreManager* scoreManager) { _scoreManager = scoreManager; }
    ScoreManager* getScoreManager() const { return _scoreManager; }
    int getLives() const { return _scoreManager ? _scoreManager->getLives() : 0; }

private:
    LevelData _currentLevelData;
    ScoreManager* _scoreManager = nullptr;
    float _freezeTimer = 0.0f;
    bool _levelCleared = false;
    std::shared_ptr<sf::Vector2f> _checkpointPos = nullptr;
    PhysicsWorld _physicsWorld;
    GameObjectFactory _objectFactory;
    std::array<FireballPool, 2> _fireballPools;
    WorldObjectStore _objectStore;
    WorldMap _worldMap;
    WorldInteractionSystem _interactionSystem;
    WorldRenderer _renderer;
};
