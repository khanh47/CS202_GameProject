#pragma once

#include <unordered_set>
#include <vector>

#include <SFML/Graphics/Rect.hpp>

#include "Game/World/AutotileTilesetDef.h"

// ---------------------------------------------------------------------------
// AutotileResolver
//
// Stateless utility.  Given a screen-space tile ID grid and a set of tile IDs
// that are considered "solid", it samples the four orthogonal neighbours of a
// cell, builds the 4-bit bitmask, and returns the corresponding sf::IntRect
// from the supplied AutotileTilesetDef.
//
// Bit encoding (matches AutotileTilesetDef):
//   bit 0 (1) = top     solid
//   bit 1 (2) = right   solid
//   bit 2 (4) = bottom  solid
//   bit 3 (8) = left    solid
//
// Out-of-bounds rules (authentic Mario style):
//   • Below the grid   → treated as SOLID   (seamless underground floor)
//   • Above / L / R    → treated as EMPTY   (open air at edges)
// ---------------------------------------------------------------------------
class AutotileResolver {
public:
    // Returns true if the tile at (col, screenRow) is suspended in mid-air
    // (i.e. has no continuous chain of solid tiles beneath it reaching the ground).
    bool isFloating(
        const std::vector<std::vector<int>>& screenGrid,
        int col,
        int screenRow,
        int gridWidth,
        int gridHeight,
        const std::unordered_set<int>&       solidIds
    ) const;

    // Returns the texture sub-rect to use for the tile at (col, screenRow).
    //
    // screenGrid[row][col] = raw tile ID in screen space (0 = empty).
    // solidIds             = set of tile IDs that count as solid neighbours.
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
