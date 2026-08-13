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
#include "Game/UserInput/IPlayerController.h"

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
    if (_minigameResult != MinigameResult::Running) {
        return;
    }
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
    resolveMinigameFalls();

    if (_minigameResult != MinigameResult::Running) {
        return;
    }

    if (_levelCleared) {
        _objectStore.cleanupDestroyed();
        return;
    }

    _objectStore.finalizeSimulation(fixedDt);
    _objectStore.cleanupDestroyed();
}

void GameWorld::updateVisuals(float deltaTime) {
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
#include "Audio/MusicManager.h"
#include "Game/UserInput/PlayerController.h"

void GameWorld::loadMap(const LevelData& levelData) {
    _currentLevelData = levelData;
    _levelCleared = false;
    _minigameResult = MinigameResult::Running;
    _worldMap.rebuild(
        levelData,
        _physicsWorld,
        _objectFactory,
        _fireballPools,
        _objectStore,
        *this
    );
}

void GameWorld::addController(
    std::unique_ptr<IPlayerController> controller
) {
    _objectStore.addController(std::move(controller));
}

std::shared_ptr<Player> GameWorld::getPlayer(PlayerSlot slot) const {
    return _objectStore.getPlayer(slot);
}

void GameWorld::reportPlayerStomp(Player& winner, Player& loser) {
    if (GameSettings::getInstance().gameMode != GameMode::Minigame
        || _minigameResult != MinigameResult::Running
        || &winner == &loser) {
        return;
    }

    if (winner.getPlayerSlot() == PlayerSlot::One
        && loser.getPlayerSlot() == PlayerSlot::Two) {
        _minigameResult = MinigameResult::PlayerOneWon;
    } else if (winner.getPlayerSlot() == PlayerSlot::Two
               && loser.getPlayerSlot() == PlayerSlot::One) {
        _minigameResult = MinigameResult::PlayerTwoWon;
    }
}

void GameWorld::finishMinigameAsTimeout() {
    if (_minigameResult == MinigameResult::Running) {
        _minigameResult = MinigameResult::Timeout;
    }
}

void GameWorld::resolveMinigameFalls() {
    if (GameSettings::getInstance().gameMode != GameMode::Minigame
        || _minigameResult != MinigameResult::Running) {
        return;
    }

    const std::shared_ptr<Player> playerOne = getPlayer(PlayerSlot::One);
    const std::shared_ptr<Player> playerTwo = getPlayer(PlayerSlot::Two);
    if (!playerOne || !playerTwo) {
        return;
    }

    const sf::FloatRect bounds = getBounds();
    const float fallBoundary = bounds.position.y + bounds.size.y
        + _worldMap.getCellSize();
    const bool playerOneFell = playerOne->getPosition().y > fallBoundary;
    const bool playerTwoFell = playerTwo->getPosition().y > fallBoundary;
    if (playerOneFell && playerTwoFell) {
        _minigameResult = MinigameResult::Draw;
    } else if (playerOneFell) {
        _minigameResult = MinigameResult::PlayerTwoWon;
        playerOne->destroy();
    } else if (playerTwoFell) {
        _minigameResult = MinigameResult::PlayerOneWon;
        playerTwo->destroy();
    }
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

                    sf::Vector2f spawnPosition = {
                        cellPosition.x + spec.offset.x,
                        cellPosition.y + spec.offset.y + verticalOffset
                    };

                    if(_checkpointPos) spawnPosition = (*_checkpointPos);

                    auto player = _objectFactory.createPlayer(spec.typeKey, &texture, spec.animationId);
                    player->spawn(_physicsWorld, spawnPosition, spec.size);

                    if (auto mario = std::dynamic_pointer_cast<Player>(player)) {
                        mario->changeToNormalState();
                        mario->setGameWorld(*this);
                        mario->setPlayerSlot(spec.playerSlot);

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
                    player->addBehaviour<Invincible>(2.0f);
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
