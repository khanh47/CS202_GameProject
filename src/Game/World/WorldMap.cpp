#include "Game/World/WorldMap.h"

#include <algorithm>
#include <memory>

#include "Game/Objects/Block/Block.h"
#include "Game/Objects/GameObject.h"
#include "Game/Objects/GameObjectFactory.h"
#include "Game/Objects/Projectile/FireballPool.h"
#include "Game/World/GameWorld.h"
#include "Game/World/PrefabSpawner.h"
#include "Physics/CollisionFilter.h"
#include "Physics/PhysicsUnits.h"
#include "Physics/PhysicsWorld.h"
#include "ResourceManager.h"

WorldMap::WorldMap(int gridWidth, int gridHeight, float cellSize)
    : _cellSize(cellSize),
      _gridWidth(gridWidth),
      _gridHeight(gridHeight) {}

WorldMap::~WorldMap() {
    destroyBoundaryWalls();
    destroyTileCollisionObjects();
}

void WorldMap::rebuild(
    const LevelData& levelData,
    PhysicsWorld& physicsWorld,
    GameObjectFactory& objectFactory,
    std::array<FireballPool, 2>& fireballPools,
    WorldObjectStore& objectStore,
    GameWorld& gameWorld
) {
    const std::vector<std::string>& layer = levelData.layer;

    auto& resources = ResourceManager::getInstance();
    sf::Texture& itemsTexture = resources.getTexture("mario_and_items");

    _background = levelData.background;
    objectStore.clear();
    for (FireballPool& pool : fireballPools) {
        pool.initialize(physicsWorld, itemsTexture);
    }

    _terrainSeamFilter.clear();
    destroyBoundaryWalls();
    destroyTileCollisionObjects();
    _terrainSeamFilter.install(physicsWorld);

    _loadedRows = std::min(
        static_cast<int>(layer.size()),
        _gridHeight
    );
    _loadedColumns = 0;
    for (int row = 0; row < _loadedRows; ++row) {
        _loadedColumns = std::max(
            _loadedColumns,
            std::min(static_cast<int>(layer[row].size()), _gridWidth)
        );
    }

    _tileMap.initialize(_gridWidth, _gridHeight, _cellSize);
    createBoundaryWalls(physicsWorld);
    _terrainSeamFilter.setBoundaryColumns(-1, _loadedColumns);

    PrefabSpawner spawner(
        physicsWorld,
        objectFactory,
        objectStore,
        gameWorld,
        _terrainSeamFilter,
        _cellSize
    );

    // The single dense layer can describe either a static tile or a live
    // object. The mapped prefab decides which path the cell takes.
    for (int mapRow = 0; mapRow < _loadedRows; ++mapRow) {
        const int screenRow = screenYForMapRow(mapRow);
        const int columns = std::min(
            static_cast<int>(layer[mapRow].size()),
            _gridWidth
        );

        for (int column = 0; column < columns; ++column) {
            const char symbol = layer[mapRow][column];
            const auto mappingIt = levelData.tileMapping.find(symbol);
            if (mappingIt == levelData.tileMapping.end()) {
                continue;
            }

            const SpawnSpec spec = levelData.prefabs.resolve(
                mappingIt->second
            );
            const sf::Vector2f cellCenter = mapCellCenter(column, mapRow);

            if (!spec.objectKind) {
                sf::Texture* texture = nullptr;
                if (!spec.textureKey.empty()) {
                    texture = &resources.getTexture(spec.textureKey);
                }
                _tileMap.setTile(column, screenRow, symbol, texture);

                if (spec.solid) {
                    createTileCollision(
                        physicsWorld,
                        column,
                        screenRow
                    );
                }
                continue;
            }

            spawner.spawnAtGrid(
                spec,
                column,
                screenRow,
                cellCenter
            );
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

int WorldMap::screenYForMapRow(int mapRow) const noexcept {
    return screenRowFor(mapRow, _loadedRows, _gridHeight);
}

void WorldMap::createTileCollision(
    PhysicsWorld& physicsWorld,
    int column,
    int screenRow
) {
    // Keep one static body per occupied cell. Besides making tile collision
    // ownership explicit, this lets TerrainSeamFilter remove internal seams
    // exactly at the same coordinates as the dense layer.
    auto collisionTile = std::make_shared<Block>();
    collisionTile->spawn(
        physicsWorld,
        {
            column * _cellSize + _cellSize * 0.5f,
            screenRow * _cellSize + _cellSize * 0.5f
        },
        {_cellSize, _cellSize}
    );
    _terrainSeamFilter.addBlock(
        collisionTile,
        column,
        screenRow,
        column * _cellSize,
        (column + 1) * _cellSize
    );
    _tileCollisionObjects.push_back(std::move(collisionTile));
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

    for (float edgeX : {
             -_cellSize * 0.25f,
             rightEdge + _cellSize * 0.25f
         }) {
        wallDef.position = PhysicsUnits::toMeters({edgeX, midY});
        const b2BodyId body = b2CreateBody(physicsWorld.getId(), &wallDef);
        b2CreatePolygonShape(body, &shapeDef, &box);
        _boundaryWalls.push_back(body);
    }
}

void WorldMap::destroyTileCollisionObjects() {
    _tileCollisionObjects.clear();
}

void WorldMap::destroyBoundaryWalls() {
    for (const b2BodyId body : _boundaryWalls) {
        if (b2Body_IsValid(body)) {
            b2DestroyBody(body);
        }
    }
    _boundaryWalls.clear();
}
