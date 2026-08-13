#include "Game/World/AutotileResolver.h"

bool AutotileResolver::isFloating(
    const std::vector<std::vector<int>>& screenGrid,
    int col,
    int screenRow,
    int gridWidth,
    int gridHeight,
    const std::unordered_set<int>&       solidIds
) const {
    // Trace downwards from cell below screenRow to the bottom of the map grid
    int r = screenRow + 1;
    while (r < gridHeight) {
        if (r >= static_cast<int>(screenGrid.size())) {
            // Below loaded level rows -> touches map floor (not floating)
            return false;
        }
        const auto& rowData = screenGrid[r];
        if (col < 0 || col >= static_cast<int>(rowData.size())) {
            return true;
        }
        const int id = rowData[col];
        if (id == 0 || solidIds.count(id) == 0) {
            // Hit empty space before reaching bottom floor -> FLOATING IN MID-AIR!
            return true;
        }
        r++;
    }
    // Reached bottom of grid -> ground connected
    return false;
}

sf::IntRect AutotileResolver::resolve(
    const std::vector<std::vector<int>>& screenGrid,
    int col,
    int screenRow,
    int gridWidth,
    int gridHeight,
    const std::unordered_set<int>&       solidIds,
    const AutotileTilesetDef&            def
) const {
    const bool top    = isSolid(screenGrid, col,     screenRow - 1,
                                gridWidth, gridHeight, solidIds);
    const bool right  = isSolid(screenGrid, col + 1, screenRow,
                                gridWidth, gridHeight, solidIds);
    const bool bottom = isSolid(screenGrid, col,     screenRow + 1,
                                gridWidth, gridHeight, solidIds);
    const bool left   = isSolid(screenGrid, col - 1, screenRow,
                                gridWidth, gridHeight, solidIds);

    const int mask = (top    ? 1 : 0)
                   | (right  ? 2 : 0)
                   | (bottom ? 4 : 0)
                   | (left   ? 8 : 0);

    sf::IntRect rect = def.maskToRect[mask];

    // For custom 16x16 tilesets (like NSMB grassland/castle/underground),
    // alternate between Variation A (col 1, x=16) and Variation B (col 2, x=32)
    // based on (col % 2) for flat top surfaces, inner dirt, and bottom borders.
    // This creates a smooth, continuous wavy grass canopy and matching dirt waves!
    if ((col % 2) != 0 && rect.size.x == 16 && rect.position.x == 16) {
        rect.position.x = 32;
    }

    return rect;
}

bool AutotileResolver::isSolid(
    const std::vector<std::vector<int>>& screenGrid,
    int col,
    int screenRow,
    int gridWidth,
    int gridHeight,
    const std::unordered_set<int>&       solidIds
) const {
    // Below the grid → solid (underground floor continues past the map bottom)
    if (screenRow >= gridHeight) {
        return true;
    }

    // Outside left/right bounds or above the grid → empty (open air)
    if (screenRow < 0 || col < 0 || col >= gridWidth) {
        return false;
    }

    // Row may be shorter than gridWidth if the level data is narrower
    if (screenRow >= static_cast<int>(screenGrid.size())) {
        return false;
    }
    const std::vector<int>& row = screenGrid[screenRow];
    if (col >= static_cast<int>(row.size())) {
        return false;
    }

    const int id = row[col];
    return id != 0 && solidIds.count(id) > 0;
}
