#include "Game/World/GameWorld.h"

#include <algorithm>
#include <memory>

#include "Game/Behaviours/Animatable.h"
#include "Game/Behaviours/Invincible.h"
#include "Game/GameSettings.h"
#include "Game/Objects/Player/Player.h"
#include "Game/Objects/Pipe/Pipe.h"
#include "Game/Objects/Projectile/KoopaShell.h"
#include "Game/Objects/Enemy/ConcreteEnemy/Koopa.h"
#include "Game/World/LevelDataLoader.h"
#include "Game/World/PrefabSpawner.h"

GameWorld::GameWorld(int gridWidth, int gridHeight, float cellSize)
    : _worldMap(gridWidth, gridHeight, cellSize) {}

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

    _objectStore.updateSimulation(fixedDt);
    _physicsWorld.updateSimulation(fixedDt);

    // Events must be consumed while every fixture owner is still alive.
    handleSensors(_physicsWorld.getSensorEvents());
    handleContacts(_physicsWorld.getContactEvents());

    if (_levelCleared) {
        _objectStore.cleanupDestroyed();
        _worldMap.cleanupDestroyedTiles();
        return;
    }

    _objectStore.finalizeSimulation(fixedDt);
    _objectStore.cleanupDestroyed();
    _worldMap.cleanupDestroyedTiles();
}

void GameWorld::updateVisuals(float deltaTime) {
    _objectStore.updateVisuals(deltaTime);
    for (FireballPool& pool : _fireballPools) {
        pool.updateVisuals(deltaTime);
    }
    for (BlockBreakEffect& effect : _blockBreakEffects) {
        effect.update(deltaTime);
    }
    std::erase_if(
        _blockBreakEffects,
        [](const BlockBreakEffect& effect) { return effect.isFinished(); }
    );
}

void GameWorld::render(sf::RenderTarget& target) {
    _renderer.render(
        target,
        _worldMap,
        _objectStore,
        _fireballPools
    );
    for (const BlockBreakEffect& effect : _blockBreakEffects) {
        effect.render(target);
    }
}

void GameWorld::handleContacts(b2ContactEvents contactEvents) {
    _interactionSystem.processContacts(contactEvents);
}

void GameWorld::handleSensors(b2SensorEvents sensorEvents) {
    _interactionSystem.processSensors(sensorEvents);
}

#include "ResourceManager.h"
#include "Audio/MusicManager.h"
#include "Game/UserInput/PlayerController.h"

void GameWorld::loadMap(const LevelData& levelData) {
    _currentLevelData = levelData;
    _levelCleared = false;
    _blockBreakEffects.clear();
    _worldMap.rebuild(
        levelData,
        _physicsWorld,
        _objectFactory,
        _fireballPools,
        _objectStore,
        *this
    );
}

void GameWorld::spawnBlockBreakEffect(
    sf::Vector2f position,
    sf::Vector2f blockSize,
    const sf::Texture* texture,
    sf::IntRect textureRect
) {
    BlockBreakEffect effect;
    effect.spawn(position, blockSize, texture, textureRect);
    _blockBreakEffects.push_back(std::move(effect));
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

void GameWorld::saveCheckpoint(sf::Vector2f position) {
    _checkpointPos = make_shared<sf::Vector2f>(position);
}

void GameWorld::respawnPlayer() {
    PrefabSpawner spawner(
        _physicsWorld,
        _objectFactory,
        _objectStore,
        *this,
        _worldMap.getTerrainSeamFilter(),
        _worldMap.getCellSize()
    );

    const std::vector<std::string>& layer = _currentLevelData.layer;
    for (int mapRow = 0; mapRow < _worldMap.getLoadedRows(); ++mapRow) {
        const int columns = std::min(
            static_cast<int>(layer[mapRow].size()),
            _worldMap.getLoadedColumns()
        );
        for (int column = 0; column < columns; ++column) {
            const auto mappingIt = _currentLevelData.tileMapping.find(
                layer[mapRow][column]
            );
            if (mappingIt == _currentLevelData.tileMapping.end()) {
                continue;
            }

            const SpawnSpec spec = _currentLevelData.prefabs.resolve(
                mappingIt->second
            );
            if (!spec.objectKind || *spec.objectKind != ObjectKind::Player) {
                continue;
            }

            const sf::Vector2f cellPosition = _worldMap.mapCellCenter(
                column,
                mapRow
            );
            const std::shared_ptr<GameObject> object = _checkpointPos
                ? spawner.spawnAtPosition(spec, *_checkpointPos)
                : spawner.spawnAtGrid(
                    spec,
                    column,
                    WorldMap::screenRowFor(
                        mapRow,
                        _worldMap.getLoadedRows(),
                        _worldMap.getGridHeight()
                    ),
                    cellPosition
                );
            if (object) {
                object->addBehaviour<Invincible>(2.0f);
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

bool GameWorld::spawnKoopaShell(sf::Vector2f spawnPosition, bool facingRight) {
    sf::Texture& itemsTexture = ResourceManager::getInstance().getTexture("koopa_spritesheet");
    auto shell = std::make_shared<KoopaShell>(itemsTexture);
    shell->setFacingRight(facingRight);
    shell->setGameWorld(this);
    shell->spawn(_physicsWorld, spawnPosition, {60.0f, 48.0f});
    _objectStore.addObject(std::move(shell));
    return true;
}

bool GameWorld::spawnKoopa(sf::Vector2f spawnPosition, bool facingRight) {
    sf::Texture& itemsTexture = ResourceManager::getInstance().getTexture("koopa_spritesheet");
    // Bottom-align the koopa (100px tall) with the shell (48px tall) so it spawns resting on the ground.
    spawnPosition.y -= (100.0f - 48.0f) * 0.5f;
    auto koopa = std::make_shared<Koopa>(itemsTexture, "koopa", true);
    koopa->setGameWorld(this);
    koopa->setFacingRight(facingRight);
    koopa->setSupportGrid(&_worldMap.getTerrainSeamFilter(), _worldMap.getCellSize());
    koopa->spawn(_physicsWorld, spawnPosition, {64.0f, 100.0f});
    _objectStore.addObject(std::move(koopa));
    return true;
}

bool GameWorld::tryWarpPlayer(Player& player) {
    const sf::Vector2f playerPosition = player.getPosition();
    const sf::Vector2f playerSize = player.getHitboxPixels();
    const float halfWidth = playerSize.x * 0.5f;
    const float halfHeight = playerSize.y * 0.5f;
    const Pipe* source = nullptr;

    for (const auto& object : _objectStore.objects()) {
        const auto pipe = std::dynamic_pointer_cast<Pipe>(object);
        if (!pipe || !pipe->isWarp()) continue;
        const sf::Vector2f pipePosition = pipe->getPosition();
        const sf::Vector2f pipeSize = pipe->getHitboxPixels();
        const float left = pipePosition.x - pipeSize.x * 0.5f;
        const float right = pipePosition.x + pipeSize.x * 0.5f;
        const float top = pipePosition.y - pipeSize.y * 0.5f;
        const float bottom = pipePosition.y + pipeSize.y * 0.5f;
        const bool verticalOverlap = playerPosition.x + halfWidth > left + 4.0f
            && playerPosition.x - halfWidth < right - 4.0f;
        const bool horizontalOverlap = playerPosition.y + halfHeight > top + 4.0f
            && playerPosition.y - halfHeight < bottom - 4.0f;
        const sf::Vector2f velocity = player.getVelocity();
        bool entering = false;
        switch (pipe->getEndSide()) {
            case Pipe::EndSide::Top:
                entering = verticalOverlap && std::abs(playerPosition.y + halfHeight - top) < 20.0f && player.isMoveDownHeld();
                break;
            case Pipe::EndSide::Bottom:
                entering = verticalOverlap && std::abs(playerPosition.y - halfHeight - bottom) < 20.0f && player.isMoveUpHeld();
                break;
            case Pipe::EndSide::Left:
                entering = horizontalOverlap && std::abs(playerPosition.x + halfWidth - left) < 20.0f && velocity.x < -20.0f;
                break;
            case Pipe::EndSide::Right:
                entering = horizontalOverlap && std::abs(playerPosition.x - halfWidth - right) < 20.0f && velocity.x > 20.0f;
                break;
        }
        if (entering) {
            source = pipe.get();
            break;
        }
    }
    if (!source) return false;

    const Pipe* destination = nullptr;
    for (const auto& object : _objectStore.objects()) {
        const auto pipe = std::dynamic_pointer_cast<Pipe>(object);
        if (pipe && pipe->isWarp() && pipe.get() != source
            && source->getWarpTarget() >= 0
            && pipe->getWarpID() == source->getWarpTarget()) {
            destination = pipe.get();
            break;
        }
    }
    if (!destination) return false;

    const sf::Vector2f destinationPosition = destination->getPosition();
    const sf::Vector2f destinationSize = destination->getHitboxPixels();
    sf::Vector2f arrival = destinationPosition;
    switch (destination->getEndSide()) {
        case Pipe::EndSide::Top: arrival.y -= destinationSize.y * 0.5f + halfHeight + 4.0f; break;
        case Pipe::EndSide::Bottom: arrival.y += destinationSize.y * 0.5f + halfHeight + 4.0f; break;
        case Pipe::EndSide::Left: arrival.x -= destinationSize.x * 0.5f + halfWidth + 4.0f; break;
        case Pipe::EndSide::Right: arrival.x += destinationSize.x * 0.5f + halfWidth + 4.0f; break;
    }
    player.setPosition(arrival);
    if (auto body = player.getPhysicsBody()) {
        b2Body_SetLinearVelocity(body->getId(), {0.0f, 0.0f});
    }
    return true;
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
    } else if (itemTypeKey == "CheckpointFlag") {
        texture = &resources.getTexture("checkpoint_flag_spritesheet");
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

    if (_scoreManager) {
        _scoreManager->handleEvent(ScoreEventType::FlagpoleReached, position);
        // Play course clear music (non-looping)
        Audio::MusicManager::getInstance().setVolume(GameSettings::getInstance().musicVolume);
        Audio::MusicManager::getInstance().play("course_clear", false);
    }
}

void GameWorld::syncPlayerControllers() {
    _objectStore.syncControllersWithKeyboard();
}

void GameWorld::playVictoryAnimation() {
    for (const std::shared_ptr<GameObject>& object : _objectStore.objects()) {
        if (auto player = std::dynamic_pointer_cast<Player>(object)) {
            if (auto* animatable = player->getBehaviour<Animatable>()) {
                animatable->playAnimation("victory");
            }
        }
    }
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
