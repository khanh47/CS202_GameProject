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

    if (_levelCleared) {
        return;
    }

    _objectStore.updateSimulation(fixedDt);

    constexpr float maximumFireballDistance = 1280.0f;
    const float voidThreshold =
        _worldMap.getGridHeight() * _worldMap.getCellSize();
    for (FireballPool& pool : _fireballPools) {
        pool.updateSimulation(
            fixedDt,
            maximumFireballDistance,
            voidThreshold
        );
    }

    _physicsWorld.updateSimulation(fixedDt);

    // Events must be consumed while every fixture owner is still alive.
    handleSensors(_physicsWorld.getSensorEvents());
    handleContacts(_physicsWorld.getContactEvents());

    if (_levelCleared) {
        _objectStore.cleanupDestroyed();
        return;
    }

    _objectStore.finalizeSimulation(fixedDt);
    _objectStore.cleanupDestroyed();
}

void GameWorld::updateVisuals(float deltaTime) {
    if (_levelCleared) {
        return;
    }

    _objectStore.updateVisuals(deltaTime);
    for (FireballPool& pool : _fireballPools) {
        pool.updateVisuals(deltaTime);
    }
}

void GameWorld::render(sf::RenderTarget& target) {
    _renderer.render(
        target,
        _worldMap,
        _objectStore,
        _fireballPools
    );
}

void GameWorld::handleContacts(b2ContactEvents contactEvents) {
    _interactionSystem.processContacts(contactEvents);
}

void GameWorld::handleSensors(b2SensorEvents sensorEvents) {
    _interactionSystem.processSensors(sensorEvents);
}

#include "ResourceManager.h"
#include "Game/UserInput/PlayerController.h"

void GameWorld::loadMap(const LevelData& levelData) {
    _currentLevelData = levelData;
    _levelCleared = false;
    _flagpolePosition = {0.0f, 0.0f};
    _worldMap.rebuild(
        levelData,
        _physicsWorld,
        _objectFactory,
        _fireballPools,
        _objectStore,
        *this
    );
}

void GameWorld::loadLevel(const std::string& levelPath) {
    loadMap(LevelDataLoader::load(
        levelPath,
        _worldMap.getGridWidth() * 2,
        _worldMap.getGridHeight() * 2
    ));
    for (const std::shared_ptr<GameObject>& object : _objectStore.objects()) {
        if (auto player = std::dynamic_pointer_cast<Player>(object)) {
            player->setGameWorld(*this);
        }
    }
    for (FireballPool& pool : _fireballPools) {
        pool.setGameWorld(this);
    }
}

void GameWorld::respawnPlayer() {
    auto& resources = ResourceManager::getInstance();
    const GameSettings& settings = GameSettings::getInstance();

    const std::vector<std::vector<int>>& mapData = _currentLevelData.rows;
    const int loadedRows = std::min(static_cast<int>(mapData.size()), _worldMap.getGridHeight());

    for (int mapRow = 0; mapRow < loadedRows; ++mapRow) {
        const int columns = std::min(static_cast<int>(mapData[mapRow].size()), _worldMap.getGridWidth());
        const int screenY = WorldMap::screenRowFor(mapRow, loadedRows, _worldMap.getGridHeight());

        for (int column = 0; column < columns; ++column) {
            const int tileId = mapData[mapRow][column];
            if (tileId == 0) continue;

            auto spawnIt = _currentLevelData.spawns.find(tileId);
            if (spawnIt == _currentLevelData.spawns.end()) continue;

            for (const SpawnSpec& spec : spawnIt->second) {
                if (spec.kind == ObjectKind::Player) {
                    const bool isLuigi = spec.animationId.find("luigi") != std::string::npos;
                    const std::string specCharacter = isLuigi ? "luigi" : "mario";
                    if (settings.gameMode == GameMode::Solo && specCharacter != settings.player1Character) {
                        continue;
                    }

                    sf::Texture& texture = resources.getTexture(spec.textureKey);
                    const float verticalOffset = spec.centerVertically ? (_worldMap.getCellSize() - spec.size.y) * 0.5f : 0.0f;
                    const sf::Vector2f cellPosition = {
                        column * _worldMap.getCellSize() + _worldMap.getCellSize() * 0.5f,
                        screenY * _worldMap.getCellSize() + _worldMap.getCellSize() * 0.5f
                    };
                    const sf::Vector2f spawnPosition = {
                        cellPosition.x + spec.offset.x,
                        cellPosition.y + spec.offset.y + verticalOffset
                    };

                    auto player = _objectFactory.createPlayer(spec.typeKey, &texture, spec.animationId);
                    player->spawn(_physicsWorld, spawnPosition, spec.size);

                    if (auto mario = std::dynamic_pointer_cast<Player>(player)) {
                        mario->changeToNormalState();
                        mario->setGameWorld(*this);

                        if (spec.addController) {
                            const bool useWasd = settings.gameMode == GameMode::Solo || spec.animationId == "mario";
                            _objectStore.addController(
                                std::make_unique<PlayerController>(
                                    *mario,
                                    *this,
                                    useWasd ? PlayerController::ControlScheme::Wasd : PlayerController::ControlScheme::ArrowKeys
                                )
                            );
                        }
                    }
                    _objectStore.addObject(std::move(player));
                }
            }
        }
    }
}

bool GameWorld::spawnFireball(
    sf::Vector2f spawnPosition,
    bool facingRight,
    int playerIndex
) {
    if (playerIndex < 0
        || playerIndex >= static_cast<int>(_fireballPools.size())) {
        return false;
    }
    return _fireballPools[playerIndex].spawnFireball(
        spawnPosition,
        facingRight
    );
}

std::shared_ptr<GameObject> GameWorld::spawnItem(
    const std::string& itemTypeKey,
    sf::Vector2f position,
    sf::Vector2f size
) {
    sf::Texture* texture = nullptr;
    auto& resources = ResourceManager::getInstance();
    if (itemTypeKey == "Coin") {
        texture = &resources.getTexture("coin_spritesheet");
    } else if (itemTypeKey == "Flagpole") {
        texture = &resources.getTexture("goal_flag_spritesheet");
    } else {
        texture = &resources.getTexture("mario_and_items");
    }

    try {
        auto item = _objectFactory.createItem(itemTypeKey, texture);
        if (item) {
            item->spawn(_physicsWorld, position, size);
            _objectStore.addObject(item);
        }
        return item;
    } catch (...) {
        return nullptr;
    }
}

void GameWorld::freeze(float durationSeconds) {
    _freezeTimer = std::max(_freezeTimer, durationSeconds);
}

void GameWorld::reachFlagpole(sf::Vector2f position) {
    if (_levelCleared) {
        return;
    }

    _levelCleared = true;
    _flagpolePosition = position;

    if (_scoreManager) {
        _scoreManager->handleEvent(ScoreEventType::FlagpoleReached, position);
    }
}

void GameWorld::syncPlayerControllers() {
    _objectStore.syncControllersWithKeyboard();
}

std::shared_ptr<GameObject> GameWorld::getPrimaryPlayer() const {
    return _objectStore.getPrimaryPlayer();
}

bool GameWorld::hasLivingPlayers() const {
    return _objectStore.hasLivingPlayers();
}

sf::FloatRect GameWorld::getBounds() const {
    return _worldMap.getBounds();
}
