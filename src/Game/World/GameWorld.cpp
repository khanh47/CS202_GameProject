#include "Game/World/GameWorld.h"
#include "Game/Behaviours/Moveable.h"
#include "Game/GameSettings.h"
#include "Game/Objects/GameObject.h"
#include "Game/Objects/Player/Player.h"
#include "Game/Objects/Block/Block.h"
#include "Game/Objects/Item/FireFlower.h"
#include "Game/Objects/Enemy/Enemy.h"
#include "Game/UserInput/PlayerController.h"
#include "ResourceManager.h"
#include "box2d/types.h"
#include <SFML/Graphics/Texture.hpp>
#include <memory>
#include <cmath>

GameWorld::GameWorld() {
    _grid.resize(_gridHeight, std::vector<std::shared_ptr<GameObject>>(_gridWidth, nullptr));
}

GameWorld::~GameWorld() = default;

void GameWorld::handleInput(const sf::Event& event) {
    if (GameSettings::getInstance().freeCameraMove || isFrozen()) {
        return;
    }

    for (std::unique_ptr<PlayerController>& controller : _controllers) {
        if (controller && controller->handleEvent(event)) {
            break;
        }
    }
}

bool GameWorld::spawnFireball(sf::Vector2f spawnPos, bool facingRight) {
    return _fireballPool.spawnFireball(spawnPos, facingRight);
}

void GameWorld::syncPlayerControllers() {
    for (auto& controller : _controllers) {
        if (controller) {
            controller->syncStateWithKeyboard();
        }
    }
}

void GameWorld::updateSimulation(const float &fixedDt) {
    if (isFrozen()) {
        _freezeTimer -= fixedDt;
        if (_freezeTimer <= 0.0f) {
            _freezeTimer = 0.0f;
            // Synchronize player input states with physical keyboard state upon unfreezing
            syncPlayerControllers();
        }
        // Physics & object movement are paused while frozen
        return;
    }

    if (GameSettings::getInstance().freeCameraMove) {
        // Lock character controls and movement when free camera mode is active
        for (std::shared_ptr<GameObject>& object : _objects) {
            if (std::shared_ptr<Player> player = std::dynamic_pointer_cast<Player>(object)) {
                player->stopMoveLeft();
                player->stopMoveRight();
                player->stopJump();
            }
        }
    }

    finalizeGroundContacts();

    for (std::shared_ptr<GameObject>& object : _objects) {
        object->updateSimulation(fixedDt);
    }
    _physicsWorld.updateSimulation(fixedDt);

    // 1. Update simulation for pooled fireballs (1 screen width range limit & void threshold)
    const float maxDistancePixels = 1280.0f; // 1 full 1280px screen width
    const float voidYThreshold = _gridHeight * CELL_SIZE;
    _fireballPool.updateSimulation(fixedDt, maxDistancePixels, voidYThreshold);

    // 2. Fireball collisions with enemies (Goomba, Koopa) and environment obstacle blocks
    const auto& fireballs = _fireballPool.getPool();
    for (const auto& fb : fireballs) {
        if (!fb || !fb->isActive()) continue;

        // 2a. Check enemy hit (Goomba, Koopa)
        bool hitEnemy = false;
        for (auto& obj : _objects) {
            if (!obj || obj->isPendingDestroy()) continue;
            if (auto enemy = std::dynamic_pointer_cast<Enemy>(obj)) {
                sf::Vector2f enemyPos = enemy->getPosition();
                sf::Vector2f fbPos = fb->getPosition();
                const float fbRadius = 19.0f;
                float dx = fbPos.x - enemyPos.x;
                float dy = fbPos.y - enemyPos.y;
                float distSq = dx * dx + dy * dy;
                const float enemyRadius = 32.0f;
                if (distSq < (fbRadius + enemyRadius) * (fbRadius + enemyRadius)) {
                    enemy->destroy(); // Defeat enemy!
                    fb->deactivate(); // Despawn fireball on impact
                    hitEnemy = true;
                    break;
                }
            }
        }

        if (hitEnemy) continue;

        // 2b. Surface & Wall collision handling matching SMB1 NES physics
        // Search 3x3 surrounding grid neighbourhood for blocks
        sf::Vector2f fbPos = fb->getPosition();
        int centerTileX = static_cast<int>(fbPos.x / CELL_SIZE);
        int centerTileY = static_cast<int>(fbPos.y / CELL_SIZE);
        int centerLogicY = _gridHeight - 1 - centerTileY;

        bool surfaceHit = false;

        for (int ly = centerLogicY - 1; ly <= centerLogicY + 1 && !surfaceHit; ++ly) {
            if (ly < 0 || ly >= _gridHeight) continue;
            for (int tx = centerTileX - 1; tx <= centerTileX + 1; ++tx) {
                if (tx < 0 || tx >= _gridWidth) continue;

                const auto& block = _grid[ly][tx];
                if (!block) continue;

                sf::Vector2f blockPos = block->getPosition();
                float dx = fbPos.x - blockPos.x;
                float dy = fbPos.y - blockPos.y;
                float absDx = std::abs(dx);
                float absDy = std::abs(dy);

                // Combined AABB half-size threshold (Fireball 19px radius + Block 32px half-width = 51px)
                const float hitThreshold = 49.0f;
                if (absDx < hitThreshold && absDy < hitThreshold) {
                    // Differentiate top face landing (ground/platform) from side face impact (walls/stairs/pipes)
                    // If fireball center is above the block center and vertical offset dominates, it hit the TOP surface
                    if (dy < 0.0f && absDy > absDx - 6.0f) {
                        // Contact on top surface while falling or level -> trigger bounce!
                        if (fb->getVelocity().y >= -1.0f) {
                            fb->triggerBounce();
                            surfaceHit = true;
                            break;
                        }
                    } else {
                        // Contact on side face or bottom ceiling face -> explode/deactivate fireball!
                        fb->deactivate();
                        surfaceHit = true;
                        break;
                    }
                }
            }
        }

        // Also check any interactive blocks in _objects that are not in _grid
        if (!surfaceHit) {
            for (auto& obj : _objects) {
                if (!obj || obj->isPendingDestroy()) continue;
                if (std::dynamic_pointer_cast<Block>(obj)) {
                    sf::Vector2f blockPos = obj->getPosition();
                    float dx = fbPos.x - blockPos.x;
                    float dy = fbPos.y - blockPos.y;
                    float absDx = std::abs(dx);
                    float absDy = std::abs(dy);

                    const float hitThreshold = 49.0f;
                    if (absDx < hitThreshold && absDy < hitThreshold) {
                        if (dy < 0.0f && absDy > absDx - 6.0f) {
                            if (fb->getVelocity().y >= -1.0f) {
                                fb->triggerBounce();
                                surfaceHit = true;
                                break;
                            }
                        } else {
                            fb->deactivate();
                            surfaceHit = true;
                            break;
                        }
                    }
                }
            }
        }
    }

    // Check item pickup collisions (e.g. FireFlower & Player)
    std::shared_ptr<Player> primaryPlayer = std::dynamic_pointer_cast<Player>(getPrimaryPlayer());
    if (primaryPlayer) {
        sf::Vector2f playerPos = primaryPlayer->getPosition();
        for (auto& obj : _objects) {
            if (!obj || obj->isPendingDestroy()) continue;
            if (auto fireFlower = std::dynamic_pointer_cast<FireFlower>(obj)) {
                sf::Vector2f flowerPos = fireFlower->getPosition();
                float dx = playerPos.x - flowerPos.x;
                float dy = playerPos.y - flowerPos.y;
                float distSq = dx * dx + dy * dy;
                // Pickup radius of 45 pixels
                if (distSq < 45.0f * 45.0f) {
                    primaryPlayer->startFireTransformation(*this, 1.0f);
                    fireFlower->destroy();
                }
            }
        }
    }

    // Clean up destroyed game objects
    _objects.erase(
        std::remove_if(_objects.begin(), _objects.end(),
            [](const std::shared_ptr<GameObject>& obj) {
                return !obj || obj->isPendingDestroy();
            }),
        _objects.end()
    );

    b2SensorEvents sensorEvents = _physicsWorld.getSensorEvents();
    b2ContactEvents contactEvents = _physicsWorld.getContactEvents();

    handleSensors(sensorEvents);
    handleContacts(contactEvents);
}

void GameWorld::handleContacts(b2ContactEvents contactEvents) {
    for (int i = 0; i < contactEvents.beginCount; ++i)
    {
        b2ContactBeginTouchEvent* event = &contactEvents.beginEvents[i];

        b2ShapeId shapeA = event->shapeIdA;
        b2ShapeId shapeB = event->shapeIdB;

        if (!b2Shape_IsValid(shapeA) || !b2Shape_IsValid(shapeB)) continue;

        GameObject* objA = static_cast<GameObject*>(b2Shape_GetUserData(shapeA));
        GameObject* objB = static_cast<GameObject*>(b2Shape_GetUserData(shapeB));
    }

    for (int i = 0; i < contactEvents.endCount; ++i)
    {
        b2ContactEndTouchEvent* event = &contactEvents.endEvents[i];
        b2ShapeId shapeA = event->shapeIdA;
        b2ShapeId shapeB = event->shapeIdB;

        if (!b2Shape_IsValid(shapeA) || !b2Shape_IsValid(shapeB)) continue;

        GameObject* objA = static_cast<GameObject*>(b2Shape_GetUserData(shapeA));
        GameObject* objB = static_cast<GameObject*>(b2Shape_GetUserData(shapeB));
    }
}

void GameWorld::handleSensors(b2SensorEvents sensorEvents) {
    for (int i = 0; i < sensorEvents.endCount; ++i)
    {
        b2SensorEndTouchEvent* event = &sensorEvents.endEvents[i];

        b2ShapeId shapeA = event->sensorShapeId;

        if (!b2Shape_IsValid(shapeA)) continue;

        GameObject* feetOwner = static_cast<GameObject*>(b2Shape_GetUserData(shapeA));

        if (Moveable* movingComponent = dynamic_cast<Moveable*>(feetOwner)) {
            movingComponent->endGroundContact();
        }
    }

    for (int i = 0; i < sensorEvents.beginCount; ++i)
    {
        b2SensorBeginTouchEvent* event = &sensorEvents.beginEvents[i];

        b2ShapeId shapeA = event->sensorShapeId;

        if (!b2Shape_IsValid(shapeA)) continue;

        GameObject* feetOwner = static_cast<GameObject*>(b2Shape_GetUserData(shapeA));

        if (Moveable* movingComponent = dynamic_cast<Moveable*>(feetOwner)) {
            movingComponent->beginGroundContact();
        }
    }
}

void GameWorld::finalizeGroundContacts() {
    for (std::shared_ptr<GameObject>& object : _objects) {
        if (Moveable* movingComponent = dynamic_cast<Moveable*>(object.get())) {
            movingComponent->finalizeGroundContacts();
        }
    }
}

void GameWorld::updateVisuals(float deltaTime) {
    for(std::shared_ptr<GameObject> object: _objects)
        object->updateVisuals(deltaTime);

    _fireballPool.updateVisuals(deltaTime);
}

void GameWorld::render(sf::RenderTarget &target) {
    sf::View view = target.getView();

    // 1. Render static environment map tiles using Tile Culling & sf::VertexArray batching (1 draw call)
    _tileMap.updateVisibleVertices(view);
    target.draw(_tileMap);

    // 2. Perform Frustum Culling on dynamic game objects to prevent off-screen draw calls
    sf::FloatRect viewBounds(view.getCenter() - view.getSize() / 2.f, view.getSize());
    const float margin = CELL_SIZE * 2.f;
    sf::FloatRect culledBounds(
        {viewBounds.position.x - margin, viewBounds.position.y - margin},
        {viewBounds.size.x + margin * 2.f, viewBounds.size.y + margin * 2.f}
    );

    for (std::shared_ptr<GameObject> object : _objects) {
        if (!object) continue;

        // Visual rendering for static environment blocks is handled by _tileMap in batch
        if (std::dynamic_pointer_cast<Block>(object)) {
            continue;
        }

        sf::Vector2f pos = object->getPosition();
        if (culledBounds.contains(pos)) {
            object->render(target);
        }
    }

    // 3. Render active fireballs from the object pool
    _fireballPool.render(target);

    GameSettings& settings = GameSettings::getInstance();
    if (settings.debugDrawGrid || settings.debugDrawCoordinates) {
        sf::FloatRect viewBounds(view.getCenter() - view.getSize() / 2.f, view.getSize());
        
        int startX = std::max(0, static_cast<int>(viewBounds.position.x / CELL_SIZE));
        int endX = std::min(_gridWidth, static_cast<int>((viewBounds.position.x + viewBounds.size.x) / CELL_SIZE) + 1);
        
        int startY = std::max(0, static_cast<int>(viewBounds.position.y / CELL_SIZE));
        int endY = std::min(_gridHeight, static_cast<int>((viewBounds.position.y + viewBounds.size.y) / CELL_SIZE) + 1);

        if (settings.debugDrawGrid) {
            sf::VertexArray lines(sf::PrimitiveType::Lines);
            for (int x = startX; x <= endX; ++x) {
                lines.append(sf::Vertex({x * CELL_SIZE, startY * CELL_SIZE}, sf::Color::Green));
                lines.append(sf::Vertex({x * CELL_SIZE, endY * CELL_SIZE}, sf::Color::Green));
            }
            for (int y = startY; y <= endY; ++y) {
                lines.append(sf::Vertex({startX * CELL_SIZE, y * CELL_SIZE}, sf::Color::Green));
                lines.append(sf::Vertex({endX * CELL_SIZE, y * CELL_SIZE}, sf::Color::Green));
            }
            target.draw(lines);
        }

        if (settings.debugDrawCoordinates) {
            const sf::Font& font = ResourceManager::getInstance().getFont("Roboto");
            sf::Text text(font, "", 10);
            text.setFillColor(sf::Color::White);
            text.setOutlineColor(sf::Color::Black);
            text.setOutlineThickness(1.0f);

            for (int screenY = startY; screenY < endY; ++screenY) {
                for (int x = startX; x < endX; ++x) {
                    if (screenY >= _gridHeight || x >= _gridWidth) continue;
                    int logicY = _gridHeight - 1 - screenY;
                    text.setString(std::to_string(x) + "," + std::to_string(logicY));
                    text.setPosition({x * CELL_SIZE + 2.f, screenY * CELL_SIZE + 2.f});
                    target.draw(text);
                }
            }
        }
    }
}

void GameWorld::loadMap(const std::vector<std::vector<int>>& mapData) {
    sf::Texture& brickTexture = ResourceManager::getInstance().getTexture("brick");
    sf::Texture& marioTexture = ResourceManager::getInstance().getTexture("mario_spritesheet");
    sf::Texture& luigiTexture = ResourceManager::getInstance().getTexture("luigi_spritesheet");
    sf::Texture& goombaTexture = ResourceManager::getInstance().getTexture("goomba_spritesheet");
    sf::Texture& koopaTexture = ResourceManager::getInstance().getTexture("koopa_spritesheet");
    sf::Texture& itemsTexture = ResourceManager::getInstance().getTexture("mario_and_items");

    _controllers.clear();
    _objects.clear();
    _fireballPool.initialize(_physicsWorld, itemsTexture);

    _loadedRows = std::min(static_cast<int>(mapData.size()), _gridHeight);
    _loadedCols = 0;
    for (int mapY = 0; mapY < _loadedRows; ++mapY) {
        _loadedCols = std::max(_loadedCols, std::min(static_cast<int>(mapData[mapY].size()), _gridWidth));
    }

    // Initialize TileMap grid and texture binding
    _tileMap.initialize(_gridWidth, _gridHeight, CELL_SIZE);
    _tileMap.setTexture(&brickTexture);

    // Re-initialize grid based on GameWorld dimensions (500x60 default)
    _grid.assign(_gridHeight, std::vector<std::shared_ptr<GameObject>>(_gridWidth, nullptr));

    for (int mapY = 0; mapY < _loadedRows; ++mapY) {
        int cols = std::min(static_cast<int>(mapData[mapY].size()), _gridWidth);
        for (int x = 0; x < cols; ++x) {
            int blockId = mapData[mapY][x];
            if (blockId == 0) continue;

            // Map data uses standard Y-down row indexing (0 is top row of the loaded matrix).
            // Logic Y is distance from the bottom. Let's assume the provided matrix bottom row 
            // maps to logic y=1 (so it's sitting on the bottom).
            int logicY = _loadedRows - 1 - mapY + 1; // +1 to put floor 1 cell above the bottom abyss
            int screenY = _gridHeight - 1 - logicY;

            sf::Vector2f spawnPos = {
                x * CELL_SIZE + CELL_SIZE / 2.f, 
                screenY * CELL_SIZE + CELL_SIZE / 2.f
            };

            if (blockId == 1) {
                // Set tile data in TileMap for batched rendering
                _tileMap.setTile(x, screenY, blockId);

                // Brick block (ID 1) - create physics body for collisions
                auto brickBlock = _objectFactory.createBlock("Block", &brickTexture);
                // We pass CELL_SIZE because GameObject::createHitbox now correctly halves the dimensions.
                brickBlock->spawn(_physicsWorld, spawnPos, {CELL_SIZE, CELL_SIZE});
                _grid[logicY][x] = brickBlock;
                _objects.push_back(brickBlock);
            }
            else if (blockId == 2) {
                // Player 1
                auto player1 = _objectFactory.createPlayer("Player", &marioTexture, "mario");
                player1->spawn(_physicsWorld, {spawnPos.x + 10, spawnPos.y}, {96, 96}, true);
                if (auto mario = std::dynamic_pointer_cast<Player>(player1)) {
                    mario->changeToNormalState();
                    _controllers.emplace_back(std::make_unique<PlayerController>(*mario, *this, PlayerController::ControlScheme::Wasd));
                }
                _objects.push_back(player1);
            }
            // else if (blockId == 3) {
            //     // Player 2
            //     auto player2 = _objectFactory.createPlayer("Player", &luigiTexture, "luigi");
            //     player2->spawn(_physicsWorld, {spawnPos.x + 10, spawnPos.y}, {80, 80});
            //     if (auto luigi = std::dynamic_pointer_cast<Player>(player2)) {
            //         _controllers.emplace_back(std::make_unique<PlayerController>(*luigi, PlayerController::ControlScheme::ArrowKeys));
            //     }
            //     _objects.push_back(player2);
            // }
            else if (blockId == 4) {
                std::shared_ptr<GameObject> goomba = _objectFactory.createEnemy("Goomba", &goombaTexture, "goomba");
                goomba->spawn(_physicsWorld, spawnPos, sf::Vector2f(40.0f, 48.0f));
                _objects.push_back(goomba);
            }
            else if (blockId == 5) {
                std::shared_ptr<GameObject> koopa = _objectFactory.createEnemy("Koopa", &koopaTexture, "koopa");
                koopa->spawn(_physicsWorld, spawnPos, sf::Vector2f(64.0f, 80.0f));
                _objects.push_back(koopa);
            }
            else if (blockId == 6) {
                sf::Texture& itemsTexture = ResourceManager::getInstance().getTexture("mario_and_items");
                std::shared_ptr<GameObject> fireFlower = _objectFactory.createItem("FireFlower", &itemsTexture);
                const sf::Vector2f flowerSize(54.0f, 54.0f);
                const sf::Vector2f flowerPos = {
                    spawnPos.x,
                    spawnPos.y + (CELL_SIZE - flowerSize.y) * 0.5f
                };
                fireFlower->spawn(_physicsWorld, flowerPos, flowerSize);
                _objects.push_back(fireFlower);
            }
        }
    }
}

void GameWorld::test() {
    // 0 = empty, 1 = brick, 2 = player1, 3 = player2, 4 = goomba, 5 = koopa, 6 = fire flower
    // Extended 60-column map slice with ground floor, platforms, and stairs
    std::vector<std::vector<int>> mapData = {
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,00,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,00,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,00,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,00,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,00,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,00,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,00,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,00,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,1,1,0,0,00,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,1,1,0,0,00,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,00,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,00,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,2,0,0,0,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,00,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,00,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,11,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,11,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    };

    loadMap(mapData);
}

std::shared_ptr<GameObject> GameWorld::getPrimaryPlayer() const {
    // Finds and returns the first player instance in the world objects vector
    for (const auto& object : _objects) {
        if (std::dynamic_pointer_cast<Player>(object)) {
            return object;
        }
    }
    return nullptr;
}

sf::FloatRect GameWorld::getBounds() const {
    // Calculates total world bounding rectangle in pixels based on loaded columns and grid dimensions
    const float width = (_loadedCols > 0) ? _loadedCols * CELL_SIZE : _gridWidth * CELL_SIZE;
    const float height = _gridHeight * CELL_SIZE;
    return sf::FloatRect({0.0f, 0.0f}, {width, height});
}

