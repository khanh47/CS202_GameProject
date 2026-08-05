#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

// TileMap manages static map tile graphics using SFML 3 Vertex Arrays.
// Performs tile culling (frustum culling) to batch draw calls for visible tiles.
// Tiles may reference different textures; geometry is grouped into one batch
// per texture so the layer can mix multiple tile sheets.
class TileMap : public sf::Drawable {
public:
    TileMap();
    ~TileMap() override = default;

    // Resizes the internal tile grid storage
    void initialize(int gridWidth, int gridHeight, float cellSize);

    // Sets the texture for a specific grid cell (screen space col/row).
    void setTile(
        int col,
        int row,
        int tileId,
        const sf::Texture* texture
    );

    // Clears all tile data
    void clear();

    // Rebuilds the internal vertex arrays containing only visible tiles within view bounds
    void updateVisibleVertices(const sf::View& view);

    // Returns tile ID at specified grid cell
    int getTileId(int col, int row) const;

private:
    // Overridden from sf::Drawable to draw batched vertices grouped by texture
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    int _gridWidth = 0;
    int _gridHeight = 0;
    float _cellSize = 64.0f;

    struct TileInfo {
        int tileId = 0;
        const sf::Texture* texture = nullptr;
    };
    std::vector<std::vector<TileInfo>> _tiles;

    struct Batch {
        const sf::Texture* texture = nullptr;
        sf::VertexArray vertices{sf::PrimitiveType::Triangles};
    };
    std::vector<Batch> _batches;
};
