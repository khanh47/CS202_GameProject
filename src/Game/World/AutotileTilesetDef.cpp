#include "Game/World/AutotileTilesetDef.h"

#include <stdexcept>
#include <string>

AutotileTilesetDef AutotileTilesetDef::singleTile(
    const std::string& textureAlias,
    int textureWidth,
    int textureHeight
) {
    AutotileTilesetDef def;
    def.textureAlias = textureAlias;
    // {0,0,0,0} signals TileMap to sample the full texture dimensions,
    // which correctly scales 16x16 Brick.png to fit 64x64 tile cells.
    const sf::IntRect rect(
        {0, 0},
        {0, 0}
    );
    def.maskToRect.fill(rect);
    return def;
}

AutotileTilesetDef AutotileTilesetDef::rowMajor4x4(
    const std::string& textureAlias,
    int tileW,
    int tileH
) {
    AutotileTilesetDef def;
    def.textureAlias = textureAlias;
    for (int mask = 0; mask < 16; ++mask) {
        const int col = mask % 4;
        const int row = mask / 4;
        def.maskToRect[mask] = sf::IntRect(
            {col * tileW, row * tileH},
            {tileW, tileH}
        );
    }
    return def;
}

AutotileTilesetDef AutotileTilesetDef::fromJson(const nlohmann::json& json) {
    if (!json.is_object()) {
        throw std::runtime_error("AutotileTilesetDef: expected a JSON object");
    }
    if (!json.contains("texture")) {
        throw std::runtime_error(
            "AutotileTilesetDef: missing required field \"texture\""
        );
    }

    const std::string textureAlias = json["texture"].get<std::string>();
    const std::string layout       = json.value("layout", "single");
    const int         tileW        = json.value("tileWidth",  64);
    const int         tileH        = json.value("tileHeight", 64);

    if (layout == "row_major_4x4") {
        return rowMajor4x4(textureAlias, tileW, tileH);
    }

    if (layout == "custom") {
        if (!json.contains("rects") || !json["rects"].is_array()
            || json["rects"].size() != 16) {
            throw std::runtime_error(
                "AutotileTilesetDef: layout 'custom' requires a \"rects\" "
                "array with exactly 16 entries"
            );
        }
        AutotileTilesetDef def;
        def.textureAlias = textureAlias;
        const auto& rects = json["rects"];
        for (int i = 0; i < 16; ++i) {
            const auto& r = rects[i];
            if (!r.is_array() || r.size() != 4) {
                throw std::runtime_error(
                    "AutotileTilesetDef: each rect must be [x, y, w, h]"
                );
            }
            def.maskToRect[i] = sf::IntRect(
                {r[0].get<int>(), r[1].get<int>()},
                {r[2].get<int>(), r[3].get<int>()}
            );
        }
        return def;
    }

    // Default: "single" — entire texture for every mask
    return singleTile(textureAlias, tileW, tileH);
}
