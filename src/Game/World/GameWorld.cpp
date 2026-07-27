#include "Game/World/GameWorld.h"
#include "Game/GameSettings.h"
#include "Game/Objects/Player/Player.h"
#include "Game/Objects/Block/Block.h"
#include "Game/UserInput/PlayerController.h"
#include "ResourceManager.h"

GameWorld::GameWorld() {
    _grid.resize(_gridHeight, std::vector<std::shared_ptr<GameObject>>(_gridWidth, nullptr));
}

GameWorld::~GameWorld() = default;

void GameWorld::handleInput(const sf::Event& event) {
    if (GameSettings::getInstance().freeCameraMove) {
        return;
    }

    for (auto& controller : _controllers) {
        if (controller && controller->handleEvent(event)) {
            break;
        }
    }
}

void GameWorld::updateSimulation(const float &fixedDt) {
    if (GameSettings::getInstance().freeCameraMove) {
        // Lock character controls and movement when free camera mode is active
        for (auto& object : _objects) {
            if (auto player = std::dynamic_pointer_cast<Player>(object)) {
                player->stopMoveLeft();
                player->stopMoveRight();
                player->stopJump();
            }
        }
    }

    for (auto& object : _objects) {
        object->updateSimulation(fixedDt);
    }

    _physicsWorld.updateSimulation(fixedDt);
}

void GameWorld::updateVisuals(float deltaTime) {
    for(auto object: _objects)
        object->updateVisuals(deltaTime);
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

    for (auto object : _objects) {
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

    auto& settings = GameSettings::getInstance();
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
    auto& brickTexture = ResourceManager::getInstance().getTexture("brick");
    auto& marioTexture = ResourceManager::getInstance().getTexture("mario_spritesheet");
    auto& luigiTexture = ResourceManager::getInstance().getTexture("luigi_spritesheet");
    auto& goombaTexture = ResourceManager::getInstance().getTexture("goomba_spritesheet");

    _controllers.clear();
    _objects.clear();

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
                player1->spawn(_physicsWorld, {spawnPos.x + 10, spawnPos.y}, {80, 80});
                if (auto mario = std::dynamic_pointer_cast<Player>(player1)) {
                    _controllers.emplace_back(std::make_unique<PlayerController>(*mario, PlayerController::ControlScheme::Wasd));
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
                std::shared_ptr<GameObject> goomba = _objectFactory.createEnemy("Enemy", &goombaTexture, "goomba");
                goomba->spawn(_physicsWorld, spawnPos, sf::Vector2f(40.0f, 48.0f));
                _objects.push_back(goomba);
            }
        }
    }
}

void GameWorld::test() {
    // 0 = empty, 1 = brick, 2 = player1, 3 = player2, 4 = goomba,
    // Extended 60-column map slice with ground floor, platforms, and stairs
    std::vector<std::vector<int>> mapData = {
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,00,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,00,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,00,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,00,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,00,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,00,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,00,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,00,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,1,1,0,0,00,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,1,1,0,0,00,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,00,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,00,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,00,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,00,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
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

