#include "Game/World/WorldMap.h"

#include <algorithm>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include <nlohmann/json.hpp>

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

void WorldMap::loadAutotileDefs(const std::filesystem::path& path) {
    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error(
            "Unable to open autotile definitions: " + path.string()
        );
    }

    nlohmann::json document;
    input >> document;
    const auto definitionsIt = document.find("autotile_defs");
    if (definitionsIt == document.end() || !definitionsIt->is_array()) {
        throw std::runtime_error(
            "Autotile definition file must contain an autotile_defs array"
        );
    }

    _autotileDefs.clear();
    for (const nlohmann::json& definition : *definitionsIt) {
        if (!definition.is_object()
            || !definition.contains("id")
            || !definition["id"].is_string()) {
            throw std::runtime_error(
                "Every autotile definition must contain a string id"
            );
        }

        _autotileDefs.insert_or_assign(
            definition["id"].get<std::string>(),
            AutotileTilesetDef::fromJson(definition)
        );
    }
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
    _autotileDefs.clear();
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

    std::unordered_map<char, SpawnSpec> specsBySymbol;
    bool hasAutotiles = false;
    for (const auto& [symbol, prefabId] : levelData.tileMapping) {
        SpawnSpec spec = levelData.prefabs.resolve(prefabId);
        hasAutotiles = hasAutotiles || !spec.autotileId.empty();
        specsBySymbol.insert_or_assign(symbol, std::move(spec));
    }
    if (hasAutotiles) {
        loadAutotileDefs("assets/datas/autotile_defs.json");
    }

    const auto tileIdFor = [](char symbol) {
        return static_cast<int>(static_cast<unsigned char>(symbol)) + 1;
    };
    std::vector<std::vector<int>> screenGrid(
        static_cast<std::size_t>(_gridHeight),
        std::vector<int>(static_cast<std::size_t>(_gridWidth), 0)
    );
    std::unordered_set<int> solidIds;

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
            const auto specIt = specsBySymbol.find(symbol);
            if (specIt == specsBySymbol.end()) {
                continue;
            }

            const SpawnSpec& spec = specIt->second;
            const sf::Vector2f cellCenter = mapCellCenter(column, mapRow);

            const int tileId = tileIdFor(symbol);
            screenGrid[screenRow][column] = tileId;
            if (spec.solid
                || (spec.objectKind
                    && *spec.objectKind == ObjectKind::Block)) {
                solidIds.insert(tileId);
            }

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

    // Autotile definitions affect only static tiles. Dynamic blocks keep
    // their own sprites and physics bodies; static terrain remains entirely
    // owned by TileMap plus the one collision body created above.
    if (hasAutotiles) {
        // Build DSU connected-component map so resolve() can use
        // component-relative posInRow for correct wave alternation.
        _autotileResolver.precompute(screenGrid, _gridWidth, _gridHeight, solidIds);

        for (int mapRow = 0; mapRow < _loadedRows; ++mapRow) {
            const int screenRow = screenYForMapRow(mapRow);
            const int columns = std::min(
                static_cast<int>(layer[mapRow].size()),
                _gridWidth
            );

            for (int column = 0; column < columns; ++column) {
                const char symbol = layer[mapRow][column];
                const auto specIt = specsBySymbol.find(symbol);
                if (specIt == specsBySymbol.end()) {
                    continue;
                }

                const SpawnSpec& spec = specIt->second;
                if (spec.objectKind || spec.autotileId.empty()) {
                    continue;
                }

                sf::Texture* fallbackTexture = nullptr;
                if (!spec.textureKey.empty()) {
                    fallbackTexture = &resources.getTexture(
                        spec.textureKey
                    );
                }
                if (fallbackTexture == nullptr) {
                    continue;
                }

                const auto definitionIt = _autotileDefs.find(
                    spec.autotileId
                );
                if (definitionIt == _autotileDefs.end()) {
                    _tileMap.setTile(
                        column,
                        screenRow,
                        symbol,
                        fallbackTexture
                    );
                    continue;
                }

                const AutotileTilesetDef& definition = definitionIt->second;
                sf::Texture& autotileTexture = resources.getTexture(
                    definition.textureAlias
                );
                const AutotileResult autoRes = _autotileResolver.resolveDetailed(
                    screenGrid,
                    column,
                    screenRow,
                    _gridWidth,
                    _gridHeight,
                    solidIds,
                    definition
                );
                _tileMap.setTile(
                    column,
                    screenRow,
                    symbol,
                    &autotileTexture,
                    autoRes.texRect
                );
                if (autoRes.hasOverlay) {
                    _tileMap.setOverlayTile(
                        column,
                        screenRow,
                        symbol,
                        &autotileTexture,
                        autoRes.overlayRect
                    );
                }
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
