#include "Game/World/GameWorld.h"
#include "Game/GameSettings.h"
#include "Game/Behaviours/Controllable.h"

GameWorld::GameWorld() {
    _grid.resize(_gridHeight, std::vector<std::shared_ptr<GameObject>>(_gridWidth, nullptr));
}

void GameWorld::handleInput(const sf::Event& event) {
    for (auto& obj : _objects) {
        auto* controllable = dynamic_cast<Controllable*>(obj.get());
        if (controllable) {
            controllable->onInput(event);
        }
    }
}

void GameWorld::updateSimulation(const float &fixedDt) {
    _physicsWorld.updateSimulation(fixedDt);
}

void GameWorld::updateVisuals(float deltaTime) {

}

void GameWorld::render(sf::RenderTarget &target) {
    for(auto object: _objects)
        object->render(target);

    auto& settings = GameSettings::getInstance();
    if (settings.debugDrawGrid || settings.debugDrawCoordinates) {
        sf::View view = target.getView();
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

    // Re-initialize grid based on GameWorld dimensions (500x60 default)
    _grid.assign(_gridHeight, std::vector<std::shared_ptr<GameObject>>(_gridWidth, nullptr));

    int rows = std::min(static_cast<int>(mapData.size()), _gridHeight);
    for (int mapY = 0; mapY < rows; ++mapY) {
        int cols = std::min(static_cast<int>(mapData[mapY].size()), _gridWidth);
        for (int x = 0; x < cols; ++x) {
            int blockId = mapData[mapY][x];
            if (blockId == 0) continue;

            // Map data uses standard Y-down row indexing (0 is top row of the loaded matrix).
            // Logic Y is distance from the bottom. Let's assume the provided matrix bottom row 
            // maps to logic y=1 (so it's sitting on the bottom).
            int logicY = rows - 1 - mapY + 1; // +1 to put floor 1 cell above the bottom abyss
            int screenY = _gridHeight - 1 - logicY;

            sf::Vector2f spawnPos = {
                x * CELL_SIZE + CELL_SIZE / 2.f, 
                screenY * CELL_SIZE + CELL_SIZE / 2.f
            };

            if (blockId == 1) {
                // Brick block (ID 1)
                auto brickBlock = _objectFactory.createBlock("Block", &brickTexture);
                // We pass CELL_SIZE because GameObject::createHitbox now correctly halves the dimensions.
                brickBlock->spawn(_physicsWorld, spawnPos, {CELL_SIZE, CELL_SIZE});
                _grid[logicY][x] = brickBlock;
                _objects.push_back(brickBlock);
            } 
            else if (blockId == 2) {
                // Player 1
                auto player1 = _objectFactory.createPlayer();
                player1->spawn(_physicsWorld, spawnPos, {64, 123});
                _objects.push_back(player1);
            }
            else if (blockId == 3) {
                // Player 2
                auto player2 = _objectFactory.createPlayer();
                player2->spawn(_physicsWorld, spawnPos, {36, 36});
                _objects.push_back(player2);
            }
        }
    }
}

void GameWorld::test() {
    // 0 = empty, 1 = brick, 2 = player1, 3 = player2
    // A small 10x15 map slice. Bottom row is all 1s (floor).
    std::vector<std::vector<int>> mapData = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 2, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
    };

    loadMap(mapData);
}