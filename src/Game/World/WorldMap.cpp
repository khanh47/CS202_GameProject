#include "Game/World/WorldMap.h"

#include <algorithm>
#include <memory>

#include "Game/GameSettings.h"
#include "Game/Objects/GameObject.h"
#include "Game/Objects/GameObjectFactory.h"
#include "Game/Objects/Enemy/Enemy.h"
#include "Game/Objects/Enemy/ConcreteEnemy/Koopa.h"
#include "Game/Objects/Enemy/ConcreteEnemy/PiranhaPlant.h"
#include "Game/Objects/Pipe/Pipe.h"
#include "Game/Objects/Projectile/FireballPool.h"
#include "Game/Objects/Player/Player.h"
#include "Game/UserInput/PlayerController.h"
#include "Game/World/GameWorld.h"
#include "Game/World/SpawnSpec.h"
#include "Game/World/WorldObjectStore.h"
#include "Physics/CollisionFilter.h"
#include "Physics/PhysicsUnits.h"
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
            if (spec.typeKey == "Block") {
                context.tileMap.setTile(column, screenY, 1, &texture);
            }
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
            const GameSettings& settings = GameSettings::getInstance();
            const bool isLuigi = spec.animationId.find("luigi") != std::string::npos;
            const std::string specCharacter = isLuigi ? "luigi" : "mario";
            if (settings.gameMode == GameMode::Solo
                && specCharacter != settings.player1Character) {
                return;
            }

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
                mario->setPlayerSlot(spec.playerSlot);
                mario->setGameWorld(context.gameWorld);
                const bool aiOwnsPlayerTwo =
                    settings.gameMode == GameMode::Minigame
                    && settings.minigameMode == MinigameMode::VsAi
                    && spec.playerSlot == PlayerSlot::Two;
                if (spec.addController && !aiOwnsPlayerTwo) {
                    const bool useWasd =
                        settings.gameMode == GameMode::Solo
                        || spec.animationId == "mario";
                    context.objectStore.addController(
                        std::make_unique<PlayerController>(
                            *mario,
                            context.gameWorld,
                            useWasd ? PlayerController::ControlScheme::Wasd
                                    : PlayerController::ControlScheme::ArrowKeys
                        )
                    );
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
            if (auto koopa = std::dynamic_pointer_cast<Koopa>(enemy)) {
                koopa->setGameWorld(&context.gameWorld);
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
        case ObjectKind::Pipe: {
            auto pipe = context.objectFactory.createPipe(
                spec.typeKey,
                &texture,
                spec.pipeOrientation,
                spec.pipeEndSide,
                spec.pipeBodyLength,
                spec.pipeIsWarp,
                spec.warpID,
                spec.warpTarget
            );

            // Auto-compute hitbox size from pipe configuration so changing
            // pipeBodyLength alone adjusts both visuals and physics correctly.
            constexpr float pipeTileSize = 64.0f;
            const int mainAxisTiles = 1 + std::max(spec.pipeBodyLength, 0);
            sf::Vector2f pipeSize;
            if (spec.pipeOrientation == "vertical") {
                pipeSize = {2.0f * pipeTileSize,
                            static_cast<float>(mainAxisTiles) * pipeTileSize};
            } else {
                pipeSize = {static_cast<float>(mainAxisTiles) * pipeTileSize,
                            2.0f * pipeTileSize};
            }

            // A Pipe marker denotes the top-left cell of its grid footprint.
            // Its cross-axis is two cells wide, so move its centre by half a
            // cell along that axis. This makes the Box2D hitbox and the
            // occupied cells below cover exactly the same grid cells.
            sf::Vector2f pipeSpawnPos = spawnPosition;
            const float halfCell = context.cellSize * 0.5f;
            if (spec.pipeOrientation == "vertical") {
                pipeSpawnPos.x += halfCell;

                // The marker identifies the bottom cell for a top-ended pipe
                // and the top cell for a bottom-ended pipe.
                if (spec.pipeEndSide == "bottom") {
                    pipeSpawnPos.y = cellPosition.y - halfCell + pipeSize.y * 0.5f;
                } else {
                    pipeSpawnPos.y = cellPosition.y + halfCell - pipeSize.y * 0.5f;
                }
            } else if (spec.pipeOrientation == "horizontal") {
                pipeSpawnPos.y += halfCell;

                // The marker identifies the left/right end cell according to
                // the pipe orientation along its main axis.
                if (spec.pipeEndSide == "right") {
                    pipeSpawnPos.x = cellPosition.x - halfCell + pipeSize.x * 0.5f;
                } else {
                    pipeSpawnPos.x = cellPosition.x + halfCell - pipeSize.x * 0.5f;
                }
            }

            pipe->spawn(context.physicsWorld, pipeSpawnPos, pipeSize);

            if (spec.contents && spec.contents->kind == ObjectKind::Enemy) {
                const SpawnSpec& content = *spec.contents;
                auto enemy = context.objectFactory.createEnemy(
                    content.typeKey,
                    &ResourceManager::getInstance().getTexture(content.textureKey),
                    content.animationId
                );

                sf::Vector2f contentSpawnPos = pipeSpawnPos + content.offset;
                if (spec.pipeOrientation == "vertical"
                    && spec.pipeEndSide == "top") {
                    const float pipeTopY = pipeSpawnPos.y - pipeSize.y * 0.5f;
                    // Keep the hidden hitbox completely below the pipe rim,
                    // and stop the rise with its bottom exactly at that rim.
                    constexpr float hiddenDepthPixels = 8.0f;
                    const float hiddenY = pipeTopY + content.size.y * 0.5f
                        + hiddenDepthPixels;
                    const float emergedY = pipeTopY - content.size.y * 0.5f;
                    contentSpawnPos.y = spec.contentsStatic
                        ? emergedY
                        : hiddenY + content.offset.y;
                    enemy->spawn(context.physicsWorld, contentSpawnPos, content.size);

                    if (!spec.contentsStatic) {
                        if (auto plant = std::dynamic_pointer_cast<PiranhaPlant>(enemy)) {
                        plant->setPipeTravel(
                            hiddenY + content.offset.y,
                            emergedY
                        );
                        }
                    }
                } else {
                    enemy->spawn(context.physicsWorld, contentSpawnPos, content.size);
                }
                context.objectStore.addObject(std::move(enemy));
            }

            if (spec.addSeamFilter) {
                context.terrainSeamFilter.addBlock(
                    pipe,
                    column,
                    screenY,
                    column * context.cellSize,
                    (column + 1) * context.cellSize
                );

                // The pipe body spans more cells than its map spawn marker.
                // Register its complete footprint so enemies can detect the
                // pipe with their forward grid probe and turn around.
                const int pipeTiles = 1 + std::max(spec.pipeBodyLength, 0);
                if (spec.pipeOrientation == "vertical") {
                    const int firstRow = spec.pipeEndSide == "bottom"
                        ? screenY
                        : screenY - pipeTiles + 1;
                    for (int row = firstRow; row < firstRow + pipeTiles; ++row) {
                        context.terrainSeamFilter.addOccupiedCell(pipe, column, row);
                        context.terrainSeamFilter.addOccupiedCell(pipe, column + 1, row);
                    }
                } else if (spec.pipeOrientation == "horizontal") {
                    const int firstColumn = spec.pipeEndSide == "right"
                        ? column
                        : column - pipeTiles + 1;
                    for (int pipeColumn = firstColumn;
                         pipeColumn < firstColumn + pipeTiles;
                         ++pipeColumn) {
                        context.terrainSeamFilter.addOccupiedCell(
                            pipe, pipeColumn, screenY
                        );
                        context.terrainSeamFilter.addOccupiedCell(
                            pipe, pipeColumn, screenY + 1
                        );
                    }
                }
            }
            object = std::move(pipe);
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
    std::array<FireballPool, 2>& fireballPools,
    WorldObjectStore& objectStore,
    GameWorld& gameWorld
) {
    const std::vector<std::vector<int>>& mapData = levelData.rows;

    auto& resources = ResourceManager::getInstance();
    sf::Texture& itemsTexture = resources.getTexture("mario_and_items");

    _background = levelData.background;
    objectStore.clear();
    for (FireballPool& pool : fireballPools) {
        pool.initialize(physicsWorld, itemsTexture);
    }
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

    destroyBoundaryWalls();
    createBoundaryWalls(physicsWorld);
    _terrainSeamFilter.setBoundaryColumns(-1, _loadedColumns);

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

void WorldMap::createBoundaryWalls(PhysicsWorld& physicsWorld) {
    b2BodyDef wallDef = b2DefaultBodyDef();
    wallDef.type = b2_staticBody;

    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.filter.categoryBits = CollisionFilter::ENV;
    shapeDef.enableContactEvents = false;
    shapeDef.enableSensorEvents = false;

    const b2Polygon box = b2MakeBox(
        PhysicsUnits::toMeters(_cellSize * 0.25f),
        PhysicsUnits::toMeters(_gridHeight * _cellSize * 0.5f)
    );

    const float rightEdge = _loadedColumns * _cellSize;
    const float midY = _gridHeight * _cellSize * 0.5f;

    for (float edgeX : {-_cellSize * 0.25f, rightEdge + _cellSize * 0.25f}) {
        wallDef.position = PhysicsUnits::toMeters({edgeX, midY});
        b2BodyId body = b2CreateBody(physicsWorld.getId(), &wallDef);
        b2CreatePolygonShape(body, &shapeDef, &box);
        _boundaryWalls.push_back(body);
    }
}

void WorldMap::destroyBoundaryWalls() {
    for (b2BodyId body : _boundaryWalls) {
        if (b2Body_IsValid(body)) {
            b2DestroyBody(body);
        }
    }
    _boundaryWalls.clear();
}
