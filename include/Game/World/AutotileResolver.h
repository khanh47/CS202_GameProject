#pragma once

#include <unordered_set>
#include <vector>

#include <SFML/Graphics/Rect.hpp>

#include "Game/World/AutotileTilesetDef.h"

struct AutotileResult {
    sf::IntRect texRect;
    bool isSlope = false;
    int slopeType = 0; // 25 = UpRight slope, 27 = DownRight slope
};

class AutotileResolver {
public:
    // Resolves detailed tile info including slope detection and texture rect
    AutotileResult resolveDetailed(
        const std::vector<std::vector<int>>& screenGrid,
        int col,
        int screenRow,
        int gridWidth,
        int gridHeight,
        const std::unordered_set<int>&       solidIds,
        const AutotileTilesetDef&            def
    ) const;

    // Returns the texture sub-rect to use for the tile at (col, screenRow).
    sf::IntRect resolve(
        const std::vector<std::vector<int>>& screenGrid,
        int col,
        int screenRow,
        int gridWidth,
        int gridHeight,
        const std::unordered_set<int>&       solidIds,
        const AutotileTilesetDef&            def
    ) const;

private:
    bool isSolid(
        const std::vector<std::vector<int>>& screenGrid,
        int col,
        int screenRow,
        int gridWidth,
        int gridHeight,
        const std::unordered_set<int>&       solidIds
    ) const;
};
