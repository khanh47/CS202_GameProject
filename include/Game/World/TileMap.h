#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

// TileMap manages static map tile graphics using SFML 3 Vertex Arrays.
// Performs tile culling (frustum culling) to batch draw calls for visible tiles.
class TileMap : public sf::Drawable {
public:
    TileMap();
    ~TileMap() override = default;

    // Resizes the internal tile grid storage
    void initialize(int gridWidth, int gridHeight, float cellSize);

    // Binds the primary tileset texture used for batch rendering
    void setTexture(const sf::Texture* texture);

    // Sets tile ID and texture coordinates for a specific grid cell (screen space col/row)
    void setTile(int col, int row, int tileId, const sf::IntRect& texRect = sf::IntRect());

    // Clears all tile data
    void clear();

    // Rebuilds the internal vertex array containing only visible tiles within view bounds
    void updateVisibleVertices(const sf::View& view);

    // Returns tile ID at specified grid cell
    int getTileId(int col, int row) const;

private:
    // Overridden from sf::Drawable to draw batched vertices in a single pass
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    int _gridWidth = 0;
    int _gridHeight = 0;
    float _cellSize = 64.0f;
    const sf::Texture* _texture = nullptr;

    struct TileInfo {
        int tileId = 0;
        sf::IntRect texRect;
    };
    std::vector<std::vector<TileInfo>> _tiles;

    // Mutable vertex array allowing cached vertex updates during const draw/culling operations
    mutable sf::VertexArray _vertices{sf::PrimitiveType::Triangles};
};
