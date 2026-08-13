#pragma once

#include <SFML/Graphics.hpp>
#include <array>
#include <box2d/box2d.h>
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Game/Objects/Projectile/FireballPool.h"
#include "Game/World/AutotileResolver.h"
#include "Game/World/AutotileTilesetDef.h"
#include "Game/World/LevelDataLoader.h"
#include "Game/World/TerrainSeamFilter.h"
#include "Game/World/TileMap.h"

class GameObject;
class GameObjectFactory;
class GameWorld;
class PhysicsWorld;
class WorldObjectStore;

class WorldMap {
public:
    static constexpr float defaultCellSize = 64.0f;
    static constexpr int defaultGridWidth = 500;
    static constexpr int defaultGridHeight = 60;

    WorldMap(
        int gridWidth = defaultGridWidth,
        int gridHeight = defaultGridHeight,
        float cellSize = defaultCellSize
    );

    void rebuild(
        const LevelData& levelData,
        PhysicsWorld& physicsWorld,
        GameObjectFactory& objectFactory,
        std::array<FireballPool, 2>& fireballPools,
        WorldObjectStore& objectStore,
        GameWorld& gameWorld
    );

    const TerrainSeamFilter& getTerrainSeamFilter() const noexcept {
        return _terrainSeamFilter;
    }

    void renderTiles(sf::RenderTarget& target);
    sf::FloatRect getBounds() const;
    sf::Vector2f mapCellCenter(int column, int mapRow) const;
    static int screenRowFor(
        int mapRow,
        int loadedRows,
        int gridHeight
    ) noexcept {
        return gridHeight - (loadedRows - mapRow);
    }

    int getGridWidth() const noexcept { return _gridWidth; }
    int getGridHeight() const noexcept { return _gridHeight; }
    float getCellSize() const noexcept { return _cellSize; }
    int getLoadedRows() const noexcept { return _loadedRows; }
    int getLoadedColumns() const noexcept { return _loadedColumns; }
    const std::string& getBackground() const noexcept { return _background; }

private:
    int logicYForMapRow(int mapRow) const noexcept;
    int screenYForMapRow(int mapRow) const noexcept;

    void createBoundaryWalls(PhysicsWorld& physicsWorld);
    void destroyBoundaryWalls();
    void loadAutotileDefs(const std::filesystem::path& filePath);

    float _cellSize;
    int _gridWidth;
    int _gridHeight;
    int _loadedColumns = 0;
    int _loadedRows = 0;
    std::string _background;
    TileMap _tileMap;
    TerrainSeamFilter _terrainSeamFilter;
    std::vector<b2BodyId> _boundaryWalls;
    // Autotiling
    AutotileResolver _autotileResolver;
    std::unordered_map<std::string, AutotileTilesetDef> _autotileDefs;
};
