#pragma once

#include <SFML/Graphics.hpp>
#include <array>
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
    void respawnPlayer();
    void reachFlagpole(sf::Vector2f position);

    bool spawnFireball(sf::Vector2f spawnPos, bool facingRight, int playerIndex);
    bool spawnKoopaShell(sf::Vector2f spawnPos, bool facingRight);
    std::shared_ptr<GameObject> spawnItem(const std::string& itemTypeKey, sf::Vector2f position, sf::Vector2f size = {54.0f, 54.0f});
    void freeze(float durationSeconds);
    bool isFrozen() const { return _freezeTimer > 0.0f; }
    void syncPlayerControllers();

    int getGridWidth() const { return _worldMap.getGridWidth(); }
    int getGridHeight() const { return _worldMap.getGridHeight(); }
    float getCellSize() const { return _worldMap.getCellSize(); }
    std::shared_ptr<GameObject> getPrimaryPlayer() const;
    bool hasLivingPlayers() const;
    bool hasWon() const { return _levelCleared; }
    sf::Vector2f getFlagpolePosition() const { return _flagpolePosition; }
    sf::FloatRect getBounds() const;

    // Add getter & setter for ScoreManager
    void setScoreManager(ScoreManager* scoreManager) { _scoreManager = scoreManager; }
    ScoreManager* getScoreManager() const { return _scoreManager; }
    int getLives() const { return _scoreManager ? _scoreManager->getLives() : 0; }

private:
    LevelData _currentLevelData;
    ScoreManager* _scoreManager = nullptr;
    float _freezeTimer = 0.0f;
    bool _levelCleared = false;
    sf::Vector2f _flagpolePosition{0.0f, 0.0f};
    PhysicsWorld _physicsWorld;
    GameObjectFactory _objectFactory;
    std::array<FireballPool, 2> _fireballPools;
    WorldObjectStore _objectStore;
    WorldMap _worldMap;
    WorldInteractionSystem _interactionSystem;
    WorldRenderer _renderer;
};
