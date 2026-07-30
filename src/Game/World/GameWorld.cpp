#include "Game/World/GameWorld.h"

#include <algorithm>

#include "Game/GameSettings.h"
#include "Game/Objects/Player/Player.h"
#include "Game/World/LevelDataLoader.h"

GameWorld::GameWorld() = default;

GameWorld::~GameWorld() = default;

void GameWorld::handleInput(const sf::Event& event) {
    if (GameSettings::getInstance().freeCameraMove || isFrozen()) {
        return;
    }
    _objectStore.handleInput(event);
}

void GameWorld::updateSimulation(const float& fixedDt) {
    if (isFrozen()) {
        _freezeTimer -= fixedDt;
        if (_freezeTimer <= 0.0f) {
            _freezeTimer = 0.0f;
            syncPlayerControllers();
        }
        return;
    }

    if (GameSettings::getInstance().freeCameraMove) {
        _objectStore.suspendPlayerMotion();
    }

    _objectStore.finalizeGroundContacts();
    _objectStore.updateSimulation(fixedDt);

    constexpr float maximumFireballDistance = 1280.0f;
    const float voidThreshold =
        _worldMap.getGridHeight() * _worldMap.getCellSize();
    _fireballPool.updateSimulation(
        fixedDt,
        maximumFireballDistance,
        voidThreshold
    );

    _physicsWorld.updateSimulation(fixedDt);

    // Events must be consumed while every fixture owner is still alive.
    handleSensors(_physicsWorld.getSensorEvents());
    handleContacts(_physicsWorld.getContactEvents());
    _interactionSystem.processObjectInteractions(
        _objectStore,
        _fireballPool,
        *this
    );
    _objectStore.cleanupDestroyed();
}

void GameWorld::updateVisuals(float deltaTime) {
    _objectStore.updateVisuals(deltaTime);
    _fireballPool.updateVisuals(deltaTime);
}

void GameWorld::render(sf::RenderTarget& target) {
    _renderer.render(
        target,
        _worldMap,
        _objectStore,
        _fireballPool
    );
}

void GameWorld::handleContacts(b2ContactEvents contactEvents) {
    _interactionSystem.processContacts(contactEvents);
}

void GameWorld::handleSensors(b2SensorEvents sensorEvents) {
    _interactionSystem.processSensors(sensorEvents);
}

void GameWorld::loadMap(const std::vector<std::vector<int>>& mapData) {
    _worldMap.rebuild(
        mapData,
        _physicsWorld,
        _objectFactory,
        _fireballPool,
        _objectStore,
        *this
    );
}

void GameWorld::test() {
    loadMap(LevelDataLoader::load(
        "assets/levels/1-1.json",
        _worldMap.getGridWidth(),
        _worldMap.getGridHeight()
    ));
    if (auto player = std::dynamic_pointer_cast<Player>(getPrimaryPlayer())) {
        player->setGameWorld(*this);
    }
}

bool GameWorld::spawnFireball(
    sf::Vector2f spawnPosition,
    bool facingRight
) {
    return _fireballPool.spawnFireball(spawnPosition, facingRight);
}

void GameWorld::freeze(float durationSeconds) {
    _freezeTimer = std::max(_freezeTimer, durationSeconds);
}

void GameWorld::syncPlayerControllers() {
    _objectStore.syncControllersWithKeyboard();
}

std::shared_ptr<GameObject> GameWorld::getPrimaryPlayer() const {
    return _objectStore.getPrimaryPlayer();
}

sf::FloatRect GameWorld::getBounds() const {
    return _worldMap.getBounds();
}
