#include "Game/World/WorldMap.h"

#include <algorithm>
#include <memory>

#include "Game/Objects/GameObject.h"
#include "Game/Objects/GameObjectFactory.h"
#include "Game/Objects/Enemy/Enemy.h"
#include "Game/Objects/Item/FireballPool.h"
#include "Game/Objects/Player/Player.h"
#include "Game/UserInput/PlayerController.h"
#include "Game/World/GameWorld.h"
#include "Game/World/SpawnSpec.h"
#include "Game/World/WorldObjectStore.h"
#include "Physics/PhysicsWorld.h"
#include "ResourceManager.h"

namespace {
struct SpawnContext {
    PhysicsWorld& physicsWorld;
    GameObjectFactory& objectFactory;
    WorldObjectStore& objectStore;
    GameWorld& gameWorld;
    TileMap& tileMap;
    TerrainSeamFilter& terrainSeamFilter;
    float cellSize;
};

void spawnFromSpec(
    const SpawnSpec& spec,
    const SpawnContext& context,
    int column,
    int logicY,
    int screenY,
    const sf::Vector2f& cellPosition
) {
    sf::Texture& texture = ResourceManager::getInstance().getTexture(
        spec.textureKey
    );
    const float verticalOffset = spec.centerVertically
        ? (context.cellSize - spec.size.y) * 0.5f
        : 0.0f;
    const sf::Vector2f spawnPosition = {
        cellPosition.x + spec.offset.x,
        cellPosition.y + spec.offset.y + verticalOffset
    };

    std::shared_ptr<GameObject> object;
    switch (spec.kind) {
        case ObjectKind::Block: {
            auto block = context.objectFactory.createBlock(
                spec.typeKey,
                &texture
            );
            block->spawn(context.physicsWorld, spawnPosition, spec.size);
            context.tileMap.setTile(column, screenY, 1, &texture);
            if (spec.addSeamFilter) {
                context.terrainSeamFilter.addBlock(
                    block,
                    column,
                    screenY,
                    column * context.cellSize,
                    (column + 1) * context.cellSize
                );
            }
            object = std::move(block);
            break;
        }
        case ObjectKind::Player: {
            auto player = context.objectFactory.createPlayer(
                spec.typeKey,
                &texture,
                spec.animationId
            );
            player->spawn(
                context.physicsWorld,
                spawnPosition,
                spec.size
            );
            if (auto mario = std::dynamic_pointer_cast<Player>(player)) {
                mario->changeToNormalState();
                if (spec.addController) {
                    if (spec.animationId == "mario") {
                        context.objectStore.addController(
                            std::make_unique<PlayerController>(
                                *mario,
                                context.gameWorld,
                                PlayerController::ControlScheme::Wasd
                            )
                        );
                    } else {
                        context.objectStore.addController(
                            std::make_unique<PlayerController>(
                                *mario,
                                context.gameWorld,
                                PlayerController::ControlScheme::ArrowKeys
                            )
                        );
                    }
                }
            }
            object = std::move(player);
            break;
        }
        case ObjectKind::Enemy: {
            auto enemy = context.objectFactory.createEnemy(
                spec.typeKey,
                &texture,
                spec.animationId
            );
            enemy->spawn(context.physicsWorld, spawnPosition, spec.size);
            if (auto e = std::dynamic_pointer_cast<Enemy>(enemy)) {
                e->setSupportGrid(&context.terrainSeamFilter, context.cellSize);
            }
            object = std::move(enemy);
            break;
        }
        case ObjectKind::Item: {
            auto item = context.objectFactory.createItem(
                spec.typeKey,
                &texture
            );
            item->spawn(context.physicsWorld, spawnPosition, spec.size);
            object = std::move(item);
            break;
        }
    }

    context.objectStore.addObject(std::move(object));
}
}

WorldMap::WorldMap(int gridWidth, int gridHeight, float cellSize)
    : _cellSize(cellSize),
      _gridWidth(gridWidth),
      _gridHeight(gridHeight) {}

void WorldMap::rebuild(
    const LevelData& levelData,
    PhysicsWorld& physicsWorld,
    GameObjectFactory& objectFactory,
    FireballPool& fireballPool,
    WorldObjectStore& objectStore,
    GameWorld& gameWorld
) {
    const std::vector<std::vector<int>>& mapData = levelData.rows;

    auto& resources = ResourceManager::getInstance();
    sf::Texture& itemsTexture = resources.getTexture("mario_and_items");

    _background = levelData.background;
    objectStore.clear();
    fireballPool.initialize(physicsWorld, itemsTexture);
    _terrainSeamFilter.clear();
    _terrainSeamFilter.install(physicsWorld);

    _loadedRows = std::min(static_cast<int>(mapData.size()), _gridHeight);
    _loadedColumns = 0;
    for (int row = 0; row < _loadedRows; ++row) {
        _loadedColumns = std::max(
            _loadedColumns,
            std::min(static_cast<int>(mapData[row].size()), _gridWidth)
        );
    }

    _tileMap.initialize(_gridWidth, _gridHeight, _cellSize);

    SpawnContext context{
        physicsWorld,
        objectFactory,
        objectStore,
        gameWorld,
        _tileMap,
        _terrainSeamFilter,
        _cellSize
    };

    for (int mapRow = 0; mapRow < _loadedRows; ++mapRow) {
        const int columns = std::min(
            static_cast<int>(mapData[mapRow].size()),
            _gridWidth
        );
        const int logicY = logicYForMapRow(mapRow);
        const int screenY = screenYForMapRow(mapRow);

        for (int column = 0; column < columns; ++column) {
            const int tileId = mapData[mapRow][column];
            if (tileId == 0) {
                continue;
            }
            const sf::Vector2f cellPosition = {
                column * _cellSize + _cellSize * 0.5f,
                screenY * _cellSize + _cellSize * 0.5f
            };

            const auto spawnIt = levelData.spawns.find(tileId);
            if (spawnIt == levelData.spawns.end()) {
                continue;
            }
            for (const SpawnSpec& spec : spawnIt->second) {
                spawnFromSpec(
                    spec,
                    context,
                    column,
                    logicY,
                    screenY,
                    cellPosition
                );
            }
        }
    }
}

void WorldMap::renderTiles(sf::RenderTarget& target) {
    _tileMap.updateVisibleVertices(target.getView());
    target.draw(_tileMap);
}

sf::FloatRect WorldMap::getBounds() const {
    const float width = (_loadedColumns > 0 ? _loadedColumns : _gridWidth)
        * _cellSize;
    return {{0.0f, 0.0f}, {width, _gridHeight * _cellSize}};
}

sf::Vector2f WorldMap::mapCellCenter(int column, int mapRow) const {
    return {
        column * _cellSize + _cellSize * 0.5f,
        screenYForMapRow(mapRow) * _cellSize + _cellSize * 0.5f
    };
}

int WorldMap::logicYForMapRow(int mapRow) const noexcept {
    return _loadedRows - mapRow;
}

int WorldMap::screenYForMapRow(int mapRow) const noexcept {
    return screenRowFor(mapRow, _loadedRows, _gridHeight);
}
