#include "Game/World/AutotileResolver.h"

#include <numeric>   // std::iota
#include <algorithm> // std::min / std::max

// ===========================================================================
// DSU (Disjoint-Set Union) – local helper, path-compressed + union-by-rank
// ===========================================================================
namespace {

struct DSU {
    std::vector<int> parent;
    std::vector<int> rank_;

    explicit DSU(int n) : parent(n), rank_(n, 0) {
        std::iota(parent.begin(), parent.end(), 0);
    }

    int find(int x) {
        while (parent[x] != x) {
            parent[x] = parent[parent[x]]; // path halving
            x = parent[x];
        }
        return x;
    }

    void unite(int a, int b) {
        a = find(a);
        b = find(b);
        if (a == b) return;
        if (rank_[a] < rank_[b]) std::swap(a, b);
        parent[b] = a;
        if (rank_[a] == rank_[b]) ++rank_[a];
    }
};

} // anonymous namespace

// ===========================================================================
// isSolid  –  shared boundary-safe helper
// ===========================================================================
bool AutotileResolver::isSolid(
    const std::vector<std::vector<int>>& screenGrid,
    int col, int row,
    int gridWidth, int gridHeight,
    const std::unordered_set<int>& solidIds
) const noexcept {
    if (row >= gridHeight) return true;   // below the map = solid ground
    if (row < 0 || col < 0 || col >= gridWidth) return false;
    if (row >= static_cast<int>(screenGrid.size())) return false;
    const auto& rowVec = screenGrid[row];
    if (col >= static_cast<int>(rowVec.size())) return false;
    const int id = rowVec[col];
    return id != 0 && solidIds.count(id) > 0;
}

// ===========================================================================
// precompute  –  DSU component pass + per-cell info
// ===========================================================================
void AutotileResolver::precompute(
    const std::vector<std::vector<int>>& screenGrid,
    int gridWidth,
    int gridHeight,
    const std::unordered_set<int>& solidIds
) {
    _precomputedGridWidth = gridWidth;
    _cellInfo.clear();

    const int total = gridWidth * gridHeight;
    DSU dsu(total);

    // -----------------------------------------------------------------------
    // Pass 1 – union every solid cell with its right and bottom neighbours
    //   NOTE: isSolid() returns true for row >= gridHeight (virtual ground),
    //   but those cells have no DSU index, so we guard with r+1 < gridHeight.
    // -----------------------------------------------------------------------
    for (int r = 0; r < gridHeight; ++r) {
        for (int c = 0; c < gridWidth; ++c) {
            if (!isSolid(screenGrid, c, r, gridWidth, gridHeight, solidIds))
                continue;

            const int idx = r * gridWidth + c;

            // Right neighbour (c+1 < gridWidth guaranteed by isSolid check)
            if (c + 1 < gridWidth
                && isSolid(screenGrid, c + 1, r, gridWidth, gridHeight, solidIds))
            {
                dsu.unite(idx, r * gridWidth + (c + 1));
            }

            // Bottom neighbour — ONLY if within grid bounds
            if (r + 1 < gridHeight
                && isSolid(screenGrid, c, r + 1, gridWidth, gridHeight, solidIds))
            {
                dsu.unite(idx, (r + 1) * gridWidth + c);
            }
        }
    }

    // -----------------------------------------------------------------------
    // Pass 2 – fill CellInfo for every solid cell
    // -----------------------------------------------------------------------
    for (int r = 0; r < gridHeight; ++r) {
        int runStart = -1; // leftmost column of current solid horizontal run

        for (int c = 0; c <= gridWidth; ++c) {
            const bool solid = (c < gridWidth)
                && isSolid(screenGrid, c, r, gridWidth, gridHeight, solidIds);

            if (solid && runStart < 0) {
                runStart = c; // start of a new horizontal run
            }

            if (!solid && runStart >= 0) {
                // End of run: [runStart .. c-1] are all solid on this row.
                // They share the same component (we verified by union above
                // for horizontally adjacent cells).  Assign posInRow for each.
                for (int cc = runStart; cc < c; ++cc) {
                    CellInfo info;
                    info.isTopEdge    = !isSolid(screenGrid, cc, r - 1, gridWidth, gridHeight, solidIds);
                    info.isBottomEdge = !isSolid(screenGrid, cc, r + 1, gridWidth, gridHeight, solidIds);
                    info.isLeftEdge   = !isSolid(screenGrid, cc - 1, r, gridWidth, gridHeight, solidIds);
                    info.isRightEdge  = !isSolid(screenGrid, cc + 1, r, gridWidth, gridHeight, solidIds);
                    info.posInRow     = cc - runStart;   // 0 = leftmost in this run

                    _cellInfo[static_cast<std::int64_t>(r) * gridWidth + cc] = info;
                }
                runStart = -1;
            }
        }
    }
}

// ===========================================================================
// resolve  –  choose texture sub-rect using DSU component info
// ===========================================================================
sf::IntRect AutotileResolver::resolve(
    const std::vector<std::vector<int>>& screenGrid,
    int col, int screenRow,
    int gridWidth, int gridHeight,
    const std::unordered_set<int>& solidIds,
    const AutotileTilesetDef& def
) const {
    // -----------------------------------------------------------------------
    // 8-bit neighbor mask
    //   bit 0 (  1) = N    bit 1 (  2) = NE
    //   bit 2 (  4) = E    bit 3 (  8) = SE
    //   bit 4 ( 16) = S    bit 5 ( 32) = SW
    //   bit 6 ( 64) = W    bit 7 (128) = NW
    // -----------------------------------------------------------------------
    const bool N  = isSolid(screenGrid, col,     screenRow - 1, gridWidth, gridHeight, solidIds);
    const bool E  = isSolid(screenGrid, col + 1, screenRow,     gridWidth, gridHeight, solidIds);
    const bool S  = isSolid(screenGrid, col,     screenRow + 1, gridWidth, gridHeight, solidIds);
    const bool W  = isSolid(screenGrid, col - 1, screenRow,     gridWidth, gridHeight, solidIds);

    // NE/NW raw solidity (needed for top-edge connectivity rule below)
    const bool NE_solid = isSolid(screenGrid, col + 1, screenRow - 1, gridWidth, gridHeight, solidIds);
    const bool NW_solid = isSolid(screenGrid, col - 1, screenRow - 1, gridWidth, gridHeight, solidIds);

    // -----------------------------------------------------------------------
    // Top-edge connectivity rule:
    //
    // When this tile is on the TOP SURFACE of terrain (N = 0), its left/right
    // neighbours are only "connected" for grass-cap selection if they are ALSO
    // on the top surface (i.e., they have no solid tile above them).
    //
    // Without this rule, a taller adjacent column (which has solid above it)
    // would merge into the wide block's grass cap, producing a flat top-middle
    // tile instead of the expected corner tile with the grass overhang.
    //
    // Example: 3-wide block sits next to a taller thin column.  At the wide
    // block's top row the thin column is solid to the right BUT it has solid
    // above it (E's own N = NE_solid).  So maskE becomes 0, and the wide
    // block's right-top tile renders as top-right-corner (grass overhangs
    // right) rather than top-middle — exactly the NSMB look.
    // -----------------------------------------------------------------------
    const bool maskE = N ? (E && NE_solid) : E;  // express right wall edge g(8,1) down column boundary when NE_solid=0
    const bool maskW = N ? (W && NW_solid) : W;  // express left wall edge g(3,1) down column boundary when NW_solid=0

    // Diagonals are only meaningful when BOTH adjacent orthogonals are solid
    const bool NE = (N && maskE) && NE_solid;
    const bool SE = (S && maskE) && isSolid(screenGrid, col + 1, screenRow + 1, gridWidth, gridHeight, solidIds);
    const bool SW = (S && maskW) && isSolid(screenGrid, col - 1, screenRow + 1, gridWidth, gridHeight, solidIds);
    const bool NW = (N && maskW) && NW_solid;

    const int mask = (N     ?   1 : 0)
                   | (NE    ?   2 : 0)
                   | (maskE ?   4 : 0)
                   | (SE    ?   8 : 0)
                   | (S     ?  16 : 0)
                   | (SW    ?  32 : 0)
                   | (maskW ?  64 : 0)
                   | (NW    ? 128 : 0);

    sf::IntRect rect = def.maskToRect[mask];

    // Horizontal wave alternation (A -> B pattern) for inner / top-mid tiles
    int posInRow = col;
    const auto key = static_cast<std::int64_t>(screenRow) * _precomputedGridWidth + col;
    const auto it  = _cellInfo.find(key);
    if (it != _cellInfo.end()) {
        posInRow = it->second.posInRow;
    }

    if ((posInRow % 2) != 0
        && rect.size.x == 16
        && rect.position.x == 69)   // col 4: inner-A or top-mid-A
    {
        rect.position.x = 86;       // col 5: inner-B or top-mid-B
    }

    return rect;
}


// ===========================================================================
// resolveDetailed  –  slope detection + texture rect + front layer overlays
// ===========================================================================
AutotileResult AutotileResolver::resolveDetailed(
    const std::vector<std::vector<int>>& screenGrid,
    int col, int screenRow,
    int gridWidth, int gridHeight,
    const std::unordered_set<int>& solidIds,
    const AutotileTilesetDef& def
) const {
    AutotileResult result;

    // Base tile for the Back Layer (taller column wall/body)
    result.texRect = resolve(
        screenGrid, col, screenRow,
        gridWidth, gridHeight,
        solidIds, def
    );

    // -----------------------------------------------------------------------
    // Outer/Front Layer Overlay: Full Shorter Column 1-Block Expansion
    //
    // When cell (col, screenRow) is on a taller column and an adjacent column
    // is a shorter ground column, the shorter column extends 1 block toward
    // the taller one. Its complete image (top corner grass + wall body) is
    // rendered on the Front Layer (overlay), overlapping the taller column.
    // -----------------------------------------------------------------------
    const bool S = isSolid(screenGrid, col, screenRow + 1, gridWidth, gridHeight, solidIds);
    const bool W = isSolid(screenGrid, col - 1, screenRow, gridWidth, gridHeight, solidIds);
    const bool E = isSolid(screenGrid, col + 1, screenRow, gridWidth, gridHeight, solidIds);

    // 1. Check expansion from LEFT shorter column into THIS taller column
    if (W) {
        int topLeftRow = -1;
        for (int r = screenRow; r >= 0; --r) {
            if (!isSolid(screenGrid, col - 1, r - 1, gridWidth, gridHeight, solidIds)) {
                if (isSolid(screenGrid, col - 1, r, gridWidth, gridHeight, solidIds)) {
                    topLeftRow = r;
                }
                break;
            }
        }

        if (topLeftRow != -1 && screenRow >= topLeftRow) {
            // Check if THIS column is taller (solid above topLeftRow)
            if (isSolid(screenGrid, col, topLeftRow - 1, gridWidth, gridHeight, solidIds)) {
                result.hasOverlay = true;
                if (screenRow == topLeftRow) {
                    // Top-Right Corner Grass g(8,0) [137, 1]
                    result.overlayRect = def.maskToRect[80];
                } else {
                    const bool S_left = isSolid(screenGrid, col - 1, screenRow + 1, gridWidth, gridHeight, solidIds);
                    if (!S_left && !S) {
                        // Bottom-Right Corner g(8,5) [137, 86]
                        result.overlayRect = def.maskToRect[88];
                    } else {
                        // Right Wall g(8,1) [137, 18]
                        result.overlayRect = def.maskToRect[81];
                    }
                }
                return result;
            }
        }
    }

    // 2. Check expansion from RIGHT shorter column into THIS taller column
    if (E) {
        int topRightRow = -1;
        for (int r = screenRow; r >= 0; --r) {
            if (!isSolid(screenGrid, col + 1, r - 1, gridWidth, gridHeight, solidIds)) {
                if (isSolid(screenGrid, col + 1, r, gridWidth, gridHeight, solidIds)) {
                    topRightRow = r;
                }
                break;
            }
        }

        if (topRightRow != -1 && screenRow >= topRightRow) {
            // Check if THIS column is taller (solid above topRightRow)
            if (isSolid(screenGrid, col, topRightRow - 1, gridWidth, gridHeight, solidIds)) {
                result.hasOverlay = true;
                if (screenRow == topRightRow) {
                    // Top-Left Corner Grass g(3,0) [52, 1]
                    result.overlayRect = def.maskToRect[20];
                } else {
                    const bool S_right = isSolid(screenGrid, col + 1, screenRow + 1, gridWidth, gridHeight, solidIds);
                    if (!S_right && !S) {
                        // Bottom-Left Corner g(3,5) [52, 86]
                        result.overlayRect = def.maskToRect[24];
                    } else {
                        // Left Wall g(3,1) [52, 18]
                        result.overlayRect = def.maskToRect[21];
                    }
                }
                return result;
            }
        }
    }

    return result;
}
