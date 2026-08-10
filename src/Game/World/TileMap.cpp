#include "Game/World/TileMap.h"
#include <cmath>
#include <algorithm>

TileMap::TileMap() = default;

void TileMap::initialize(int gridWidth, int gridHeight, float cellSize) {
    _gridWidth = gridWidth;
    _gridHeight = gridHeight;
    _cellSize = cellSize;

    _tiles.assign(
        _gridHeight,
        std::vector<TileInfo>(_gridWidth, TileInfo{0, nullptr})
    );
}

void TileMap::setTile(
    int col,
    int row,
    int tileId,
    const sf::Texture* texture
) {
    if (row >= 0 && row < _gridHeight && col >= 0 && col < _gridWidth) {
        _tiles[row][col] = TileInfo{tileId, texture};
    }
}

void TileMap::clear() {
    _tiles.assign(
        _gridHeight,
        std::vector<TileInfo>(_gridWidth, TileInfo{0, nullptr})
    );
    _batches.clear();
}

void TileMap::updateVisibleVertices(const sf::View& view) {
    _batches.clear();
    if (_tiles.empty() || _gridWidth <= 0 || _gridHeight <= 0) {
        return;
    }

    // Debug: report view bounds and grid info
    {
        const sf::Vector2f viewCenter = view.getCenter();
        const sf::Vector2f viewSize = view.getSize();
        const sf::FloatRect viewBounds(viewCenter - viewSize / 2.f, viewSize);
    }

    // Determine the view frustum bounds in pixel coordinates
    const sf::Vector2f viewCenter = view.getCenter();
    const sf::Vector2f viewSize = view.getSize();
    const sf::FloatRect viewBounds(viewCenter - viewSize / 2.f, viewSize);

    // Padding margin around view frustum (2 cells) to avoid visual popping on edges
    const float margin = _cellSize * 2.f;

    // Tile culling index ranges clamped within map grid boundaries
    const int startCol = std::max(0, static_cast<int>(std::floor((viewBounds.position.x - margin) / _cellSize)));
    const int endCol = std::min(_gridWidth - 1, static_cast<int>(std::ceil((viewBounds.position.x + viewBounds.size.x + margin) / _cellSize)));
    const int startRow = std::max(0, static_cast<int>(std::floor((viewBounds.position.y - margin) / _cellSize)));
    const int endRow = std::min(_gridHeight - 1, static_cast<int>(std::ceil((viewBounds.position.y + viewBounds.size.y + margin) / _cellSize)));

    for (int r = startRow; r <= endRow; ++r) {
        for (int c = startCol; c <= endCol; ++c) {
            const TileInfo& tile = _tiles[r][c];
            if (tile.tileId <= 0 || tile.texture == nullptr) {
                continue;
            }

            const float tu0 = 0.f;
            const float tv0 = 0.f;
            const float tu1 = static_cast<float>(tile.texture->getSize().x);
            const float tv1 = static_cast<float>(tile.texture->getSize().y);

            sf::VertexArray* batch = nullptr;
            for (Batch& candidate : _batches) {
                if (candidate.texture == tile.texture) {
                    batch = &candidate.vertices;
                    break;
                }
            }
            if (batch == nullptr) {
                _batches.push_back(Batch{
                    tile.texture,
                    sf::VertexArray(sf::PrimitiveType::Triangles)
                });
                batch = &_batches.back().vertices;
            }

            const float left = c * _cellSize;
            const float top = r * _cellSize;
            const float right = left + _cellSize;
            const float bottom = top + _cellSize;

            // Quad construction using 2 triangles (6 vertices)
            // Triangle 1: Top-Left, Top-Right, Bottom-Right
            batch->append(sf::Vertex({left, top}, sf::Color::White, {tu0, tv0}));
            batch->append(sf::Vertex({right, top}, sf::Color::White, {tu1, tv0}));
            batch->append(sf::Vertex({right, bottom}, sf::Color::White, {tu1, tv1}));

            // Triangle 2: Top-Left, Bottom-Right, Bottom-Left
            batch->append(sf::Vertex({left, top}, sf::Color::White, {tu0, tv0}));
            batch->append(sf::Vertex({right, bottom}, sf::Color::White, {tu1, tv1}));
            batch->append(sf::Vertex({left, bottom}, sf::Color::White, {tu0, tv1}));
        }
    }

    // Debug: report how many batches were created and total vertices
    size_t totalVerts = 0;
    for (const Batch& b : _batches) totalVerts += b.vertices.getVertexCount();
}

int TileMap::getTileId(int col, int row) const {
    if (row >= 0 && row < _gridHeight && col >= 0 && col < _gridWidth) {
        return _tiles[row][col].tileId;
    }
    return 0;
}

void TileMap::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    for (const Batch& batch : _batches) {
        if (batch.texture == nullptr || batch.vertices.getVertexCount() == 0) {
            continue;
        }
        states.texture = batch.texture;
        target.draw(batch.vertices, states);
    }
}
