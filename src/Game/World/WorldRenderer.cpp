#include "Game/World/WorldRenderer.h"

#include <algorithm>
#include <cmath>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "Game/GameSettings.h"
#include "Game/Objects/Block/Block.h"
#include "Game/Objects/GameObject.h"
#include "Game/Objects/Player/Player.h"
#include "Game/Objects/Projectile/FireballPool.h"
#include "Game/World/WorldMap.h"
#include "Game/World/WorldObjectStore.h"
#include "ResourceManager.h"

namespace {
struct BackgroundLayer {
    const char* textureAlias;
    float parallaxFactor;
    float scale = 2.2f;
};

const std::vector<BackgroundLayer>& layersForBackground(
    const std::string& key
) {
    static const std::unordered_map<
        std::string,
        std::vector<BackgroundLayer>
    > registry = {
        {"parallax_sky", {
            {"far_sky", 0.2f},
            {"close_bush", 0.5f},
        }},
        {"parallax_underground", {
            {"far_underground", 0.20f},
            {"close_underground", 0.5f},
        }},
    };
    static const std::vector<BackgroundLayer> empty;
    const auto it = registry.find(key);
    return it == registry.end() ? empty : it->second;
}
}

void WorldRenderer::render(
    sf::RenderTarget& target,
    WorldMap& worldMap,
    const WorldObjectStore& objectStore,
    std::array<FireballPool, 2>& fireballPools
) const {
    renderBackground(target, worldMap.getBackground());
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
        if (auto *player = dynamic_cast<Player*>(object.get())) {
            player->render(target);
        }
    }

    for (const std::shared_ptr<GameObject>& object : objectStore.objects()) {
        if (!object || std::dynamic_pointer_cast<Block>(object) || std::dynamic_pointer_cast<Player>(object)) {
            continue;
        }
        if (culledBounds.contains(object->getPosition())) {
            object->render(target);
        }
    }

    for (FireballPool& pool : fireballPools) {
        pool.render(target);
    }
    renderDebugGrid(target, worldMap);
}

void WorldRenderer::renderBackground(
    sf::RenderTarget& target,
    const std::string& backgroundKey
) const {
    const std::vector<BackgroundLayer>& layers =
        layersForBackground(backgroundKey);
    if (layers.empty()) {
        return;
    }

    const sf::View view = target.getView();
    const sf::Vector2f viewCenter = view.getCenter();
    const sf::Vector2f viewSize = view.getSize();
    const sf::Vector2f viewPosition = viewCenter - viewSize * 0.5f;

    for (const BackgroundLayer& layer : layers) {
        sf::Texture& texture = ResourceManager::getInstance().getTexture(
            layer.textureAlias
        );
        texture.setRepeated(true);

        const float s = layer.scale;
        const sf::Vector2u texSize = texture.getSize();
        const float texW = static_cast<float>(texSize.x);
        const float texH = static_cast<float>(texSize.y);
        const float periodX = texW * s;

        const float worldOffX = std::fmod(
            std::fmod(viewCenter.x * layer.parallaxFactor, periodX) + periodX,
            periodX
        );

        sf::Sprite sprite(texture);
        sprite.setScale({s, s});
        sprite.setPosition({
            viewPosition.x - worldOffX,
            viewPosition.y
        });
        sprite.setTextureRect(sf::IntRect(
            {0, 0},
            {static_cast<int>(viewSize.x / s) + static_cast<int>(texW),
             static_cast<int>(viewSize.y / s) + static_cast<int>(texH)}
        ));
        target.draw(sprite);
    }
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

    const sf::Font& font = ResourceManager::getInstance().getFont("SuperMario");
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
