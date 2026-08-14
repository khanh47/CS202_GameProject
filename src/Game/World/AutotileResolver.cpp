#include "Game/World/AutotileResolver.h"

AutotileResult AutotileResolver::resolveDetailed(
    const std::vector<std::vector<int>>& screenGrid,
    int col,
    int screenRow,
    int gridWidth,
    int gridHeight,
    const std::unordered_set<int>&       solidIds,
    const AutotileTilesetDef&            def
) const {
    AutotileResult result;

    const bool top        = isSolid(screenGrid, col,     screenRow - 1, gridWidth, gridHeight, solidIds);
    const bool right      = isSolid(screenGrid, col + 1, screenRow,     gridWidth, gridHeight, solidIds);
    const bool bottom     = isSolid(screenGrid, col,     screenRow + 1, gridWidth, gridHeight, solidIds);
    const bool left       = isSolid(screenGrid, col - 1, screenRow,     gridWidth, gridHeight, solidIds);

    const bool top_left   = isSolid(screenGrid, col - 1, screenRow - 1, gridWidth, gridHeight, solidIds);
    const bool top_right  = isSolid(screenGrid, col + 1, screenRow - 1, gridWidth, gridHeight, solidIds);

    const bool isFlat = !top && left && right;

    // Automatic Slope Detection:
    // 1. Up-Right Slope: open top, solid right, open top-right, and not a flat surface
    if (!isFlat && !top && right && !top_right && !left) {
        result.isSlope = true;
        result.slopeType = 25; // Up-Right slope
        result.texRect = sf::IntRect({0, 208}, {16, 16});
        return result;
    }

    // 2. Down-Right Slope: open top, solid left, open top-left, and not a flat surface
    if (!isFlat && !top && left && !top_left && !right) {
        result.isSlope = true;
        result.slopeType = 27; // Down-Right slope
        result.texRect = sf::IntRect({32, 208}, {16, 16});
        return result;
    }

    // Standard autotile bitmask resolution
    result.texRect = resolve(screenGrid, col, screenRow, gridWidth, gridHeight, solidIds, def);
    result.isSlope = false;
    return result;
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
