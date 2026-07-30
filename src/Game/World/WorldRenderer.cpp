#include "Game/World/WorldRenderer.h"

#include <algorithm>
#include <memory>
#include <string>

#include "Game/GameSettings.h"
#include "Game/Objects/Block/Block.h"
#include "Game/Objects/GameObject.h"
#include "Game/Objects/Item/FireballPool.h"
#include "Game/World/WorldMap.h"
#include "Game/World/WorldObjectStore.h"
#include "ResourceManager.h"

void WorldRenderer::render(
    sf::RenderTarget& target,
    WorldMap& worldMap,
    const WorldObjectStore& objectStore,
    FireballPool& fireballPool
) const {
    worldMap.renderTiles(target);

    const sf::View view = target.getView();
    const sf::FloatRect viewBounds(
        view.getCenter() - view.getSize() * 0.5f,
        view.getSize()
    );
    const float margin = worldMap.getCellSize() * 2.0f;
    const sf::FloatRect culledBounds(
        {viewBounds.position.x - margin, viewBounds.position.y - margin},
        {viewBounds.size.x + margin * 2.0f, viewBounds.size.y + margin * 2.0f}
    );

    for (const std::shared_ptr<GameObject>& object : objectStore.objects()) {
        if (!object || std::dynamic_pointer_cast<Block>(object)) {
            continue;
        }
        if (culledBounds.contains(object->getPosition())) {
            object->render(target);
        }
    }

    fireballPool.render(target);
    renderDebugGrid(target, worldMap);
}

void WorldRenderer::renderDebugGrid(
    sf::RenderTarget& target,
    const WorldMap& worldMap
) const {
    const GameSettings& settings = GameSettings::getInstance();
    if (!settings.debugDrawGrid && !settings.debugDrawCoordinates) {
        return;
    }

    const sf::View view = target.getView();
    const sf::FloatRect viewBounds(
        view.getCenter() - view.getSize() * 0.5f,
        view.getSize()
    );
    const float cellSize = worldMap.getCellSize();
    const int startX = std::max(
        0,
        static_cast<int>(viewBounds.position.x / cellSize)
    );
    const int endX = std::min(
        worldMap.getGridWidth(),
        static_cast<int>(
            (viewBounds.position.x + viewBounds.size.x) / cellSize
        ) + 1
    );
    const int startY = std::max(
        0,
        static_cast<int>(viewBounds.position.y / cellSize)
    );
    const int endY = std::min(
        worldMap.getGridHeight(),
        static_cast<int>(
            (viewBounds.position.y + viewBounds.size.y) / cellSize
        ) + 1
    );

    if (settings.debugDrawGrid) {
        sf::VertexArray lines(sf::PrimitiveType::Lines);
        for (int x = startX; x <= endX; ++x) {
            lines.append(sf::Vertex(
                {x * cellSize, startY * cellSize},
                sf::Color::Green
            ));
            lines.append(sf::Vertex(
                {x * cellSize, endY * cellSize},
                sf::Color::Green
            ));
        }
        for (int y = startY; y <= endY; ++y) {
            lines.append(sf::Vertex(
                {startX * cellSize, y * cellSize},
                sf::Color::Green
            ));
            lines.append(sf::Vertex(
                {endX * cellSize, y * cellSize},
                sf::Color::Green
            ));
        }
        target.draw(lines);
    }

    if (!settings.debugDrawCoordinates) {
        return;
    }

    const sf::Font& font = ResourceManager::getInstance().getFont("Roboto");
    sf::Text text(font, "", 10);
    text.setFillColor(sf::Color::White);
    text.setOutlineColor(sf::Color::Black);
    text.setOutlineThickness(1.0f);
    for (int screenY = startY; screenY < endY; ++screenY) {
        for (int x = startX; x < endX; ++x) {
            const int logicY = worldMap.getGridHeight() - 1 - screenY;
            text.setString(
                std::to_string(x) + "," + std::to_string(logicY)
            );
            text.setPosition({
                x * cellSize + 2.0f,
                screenY * cellSize + 2.0f
            });
            target.draw(text);
        }
    }
}
