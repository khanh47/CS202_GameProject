#include "Game/World/GameWorld.h"
#include "Game/GameSettings.h"

GameWorld::GameWorld() {
    _grid.resize(_gridHeight, std::vector<std::shared_ptr<GameObject>>(_gridWidth, nullptr));
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

void GameWorld::test() {
    // Generate floor
    auto& brickTexture = ResourceManager::getInstance().getTexture("brick");
    for (int x = 0; x < _gridWidth; ++x) {
        int y = 1; // Floor is 1 block from the bottom logic-wise
        int screenY = _gridHeight - 1 - y;
        auto brickBlock = _objectFactory.createBlock("Block", &brickTexture);
        brickBlock->spawn(_physicsWorld, {x * CELL_SIZE + CELL_SIZE / 2.f, screenY * CELL_SIZE + CELL_SIZE / 2.f}, {CELL_SIZE, CELL_SIZE});
        _grid[y][x] = brickBlock;
        _objects.push_back(brickBlock);
    }

    // Generate some scattered platforms
    for (int i = 0; i < 5; ++i) {
        int x = 5 + i * 4;
        int y = 4 + (i % 2) * 2;
        int screenY = _gridHeight - 1 - y;
        auto platformBlock = _objectFactory.createBlock("Block", &brickTexture);
        platformBlock->spawn(_physicsWorld, {x * CELL_SIZE + CELL_SIZE / 2.f, screenY * CELL_SIZE + CELL_SIZE / 2.f}, {CELL_SIZE, CELL_SIZE});
        _grid[y][x] = platformBlock;
        _objects.push_back(platformBlock);
    }

    // Spawn players
    auto player1 = _objectFactory.createPlayer();
    int p1LogicY = 3;
    player1->spawn(_physicsWorld, {300, (_gridHeight - 1 - p1LogicY) * CELL_SIZE + CELL_SIZE / 2.f}, {64, 123});
    _objects.push_back(player1);

    auto player2 = _objectFactory.createPlayer();
    int p2LogicY = 3;
    player2->spawn(_physicsWorld, {500, (_gridHeight - 1 - p2LogicY) * CELL_SIZE + CELL_SIZE / 2.f}, {36, 36});
    _objects.push_back(player2);
}