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
    const bool maskE = N ? E : (E && !NE_solid);  // top-edge: only connect if neighbour is also top
    const bool maskW = N ? W : (W && !NW_solid);  // top-edge: only connect if neighbour is also top

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

    // -----------------------------------------------------------------------
    // Transition-tile override: NSMB-style grass extension
    //
    // When this tile is a WALL tile (N=1) and the left/right neighbour is a
    // top-surface tile (NW=0 while N+W=1, or NE=0 while N+E=1), apply the
    // special "grass drapes over the wall" tile from the tileset.
    //
    // The transition mask values are:
    //   grassFromLeft  → mask 81  (N=1 S=1 W=1 NW=0 E=0)
    //   grassFromRight → mask 21  (N=1 E=1 S=1 NE=0 W=0)
    //   flanked both   → mask 17  (N=1 S=1, treated as thin pillar mid)
    //
    // Because these masks are already defined in the JSON lookup table with
    // the correct tile rects, we just recompute the mask for the transition
    // case and re-look it up in def.maskToRect.
    // -----------------------------------------------------------------------
    if (N && S) {   // only wall/inner tiles qualify
        const bool grassFromLeft  = W && !NW_solid && !maskE;
        const bool grassFromRight = maskE && !NE_solid && !W;

        if (grassFromLeft || grassFromRight) {
            // Build a top-edge mask (N=0) so the JSON lookup returns a grass
            // cap tile rather than a wall tile.  This makes the shorter
            // column's grass cap extend one full tile into the taller column.
            //
            //   grassFromLeft  only → mask 80 (S+W)   → top-right corner g(8,0) ✓
            //   grassFromRight only → mask 20 (E+S)   → top-left  corner g(3,0) ✓
            //   both               → mask 84 (E+S+W)  → top-middle       g(4,0) ✓
            const int transitionMask =
                  (grassFromRight ? 4  : 0)   // E: connect right
                | 16                           // S: always solid below
                | (grassFromLeft  ? 64 : 0);  // W: connect left
            rect = def.maskToRect[transitionMask];
        }
    }

    // -----------------------------------------------------------------------
    // Horizontal wave alternation driven by component-relative posInRow.
    // -----------------------------------------------------------------------
    int posInRow = col;
    const auto key = static_cast<std::int64_t>(screenRow) * _precomputedGridWidth + col;
    const auto it  = _cellInfo.find(key);
    if (it != _cellInfo.end()) {
        posInRow = it->second.posInRow;
    }

    // Alternate inner / top-middle tiles between col-4 and col-5 variants
    // (A→B wave pattern).  Wall and corner tiles are NOT alternated because
    // row 2 of the tileset contains inner-corner tiles, not wall alternates.
    if ((posInRow % 2) != 0
        && rect.size.x == 16
        && rect.position.x == 69)   // col 4: inner-A or top-mid-A
    {
        rect.position.x = 86;       // col 5: inner-B or top-mid-B
    }

    return rect;
}


// ===========================================================================
// resolveDetailed  –  slope detection + texture rect
// ===========================================================================
AutotileResult AutotileResolver::resolveDetailed(
    const std::vector<std::vector<int>>& screenGrid,
    int col, int screenRow,
    int gridWidth, int gridHeight,
    const std::unordered_set<int>& solidIds,
    const AutotileTilesetDef& def
) const {
    AutotileResult result;

    // Slopes in this project are placed as dedicated autotile IDs (e.g.
    // "grassland_slope_up") rather than being auto-detected from terrain
    // neighbour patterns.  resolveDetailed therefore delegates directly to
    // resolve() for all terrain tiles.
    result.texRect = resolve(
        screenGrid, col, screenRow,
        gridWidth, gridHeight,
        solidIds, def
    );
    return result;
}
