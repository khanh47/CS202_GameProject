#pragma once

#include <array>
#include <string>
#include <SFML/Graphics/Rect.hpp>
#include <nlohmann/json.hpp>

// ---------------------------------------------------------------------------
// AutotileTilesetDef
//
// Describes one "autotile family": which texture to use and how each of the
// 16 possible 4-neighbor bitmask values maps to a sub-rectangle within that
// texture.
//
// Bitmask bit encoding (same order as TileMap neighbour probing):
//   bit 0 (value  1) = top    neighbour is solid
//   bit 1 (value  2) = right  neighbour is solid
//   bit 2 (value  4) = bottom neighbour is solid
//   bit 3 (value  8) = left   neighbour is solid
//
// maskToRect[mask] yields the sf::IntRect to pass to TileMap::setTile.
//
// ---------------------------------------------------------------------------
// HOW TO ADD A REAL TILESET
// ---------------------------------------------------------------------------
// 1. Place your PNG in  assets/sprites/Tiles/  (or any sub-folder).
//    It must contain 16 tile variants arranged in a 4 x 4 grid:
//
//       col→  0          1            2             3
//    row 0  [mask  0]  [mask  1]   [mask  2]   [mask  3]
//    row 1  [mask  4]  [mask  5]   [mask  6]   [mask  7]
//    row 2  [mask  8]  [mask  9]   [mask 10]   [mask 11]
//    row 3  [mask 12]  [mask 13]   [mask 14]   [mask 15]
//
//    Where mask = (top?1:0)|(right?2:0)|(bottom?4:0)|(left?8:0).
//    Each cell must be exactly tileWidth × tileHeight pixels.
//
// 2. Register it in ResourceManager.cpp:
//      _preLoadTexture("assets/sprites/Tiles/my_tileset.png", "my_alias");
//
// 3. Edit assets/datas/autotile_defs.json – change the entry:
//      "texture" : "my_alias"
//      "layout"  : "row_major_4x4"
//      "tileWidth"  : <px>
//      "tileHeight" : <px>
//
//    (Or keep "layout":"single" to use a single full-texture tile.)
//
// ---------------------------------------------------------------------------
struct AutotileTilesetDef {
    std::string                 textureAlias;
    std::array<sf::IntRect, 16> maskToRect{};

    // All 16 masks map to {0,0,w,h} — the whole texture.
    // Useful as a placeholder before a real tileset exists.
    static AutotileTilesetDef singleTile(
        const std::string& textureAlias,
        int textureWidth  = 64,
        int textureHeight = 64
    );

    // Masks 0-15 are laid out row-major in a 4 x 4 grid,
    // each cell being tileW x tileH pixels.
    static AutotileTilesetDef rowMajor4x4(
        const std::string& textureAlias,
        int tileW = 64,
        int tileH = 64
    );

    // Parse from a JSON object with fields:
    //   "texture"    : string alias (required)
    //   "layout"     : "single" | "row_major_4x4" | "custom"  (default: "single")
    //   "tileWidth"  : int  (used for single / row_major_4x4, default 64)
    //   "tileHeight" : int  (used for single / row_major_4x4, default 64)
    //   "rects"      : [[x,y,w,h], ...]  16 entries  (used for "custom" only)
    static AutotileTilesetDef fromJson(const nlohmann::json& json);
};
