#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>

#include "Game/World/TileMap.h"

class FireballPool;
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
        const std::vector<std::vector<int>>& mapData,
        PhysicsWorld& physicsWorld,
        GameObjectFactory& objectFactory,
        FireballPool& fireballPool,
        WorldObjectStore& objectStore,
        GameWorld& gameWorld
    );

    void renderTiles(sf::RenderTarget& target);
    sf::FloatRect getBounds() const;
    sf::Vector2f mapCellCenter(int column, int mapRow) const;
    static int screenRowFor(
        int mapRow,
        int loadedRows,
        int gridHeight
    ) noexcept {
        return gridHeight - 1 - (loadedRows - mapRow);
    }

    int getGridWidth() const noexcept { return _gridWidth; }
    int getGridHeight() const noexcept { return _gridHeight; }
    float getCellSize() const noexcept { return _cellSize; }
    int getLoadedRows() const noexcept { return _loadedRows; }
    int getLoadedColumns() const noexcept { return _loadedColumns; }

private:
    int logicYForMapRow(int mapRow) const noexcept;
    int screenYForMapRow(int mapRow) const noexcept;

    float _cellSize;
    int _gridWidth;
    int _gridHeight;
    int _loadedColumns = 0;
    int _loadedRows = 0;
    TileMap _tileMap;
    std::vector<std::vector<std::weak_ptr<GameObject>>> _grid;
};
