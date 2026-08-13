#include "Game/World/TileMap.h"
#include <cmath>
#include <algorithm>

TileMap::TileMap() = default;

void TileMap::initialize(int gridWidth, int gridHeight, float cellSize) {
    _gridWidth = gridWidth;
    _gridHeight = gridHeight;
    _cellSize = cellSize;

    const std::size_t cellCount = static_cast<std::size_t>(_gridWidth)
        * static_cast<std::size_t>(_gridHeight);
    _tiles.assign(cellCount, '.');
    _textures.assign(cellCount, nullptr);
}

void TileMap::setTile(
    int col,
    int row,
    char tileCharacter,
    const sf::Texture* texture
) {
    if (row >= 0 && row < _gridHeight && col >= 0 && col < _gridWidth) {
        const std::size_t index = static_cast<std::size_t>(row)
            * static_cast<std::size_t>(_gridWidth)
            + static_cast<std::size_t>(col);
        _tiles[index] = tileCharacter;
        _textures[index] = texture;
    }
}

void TileMap::clear() {
    _tiles.assign(
        static_cast<std::size_t>(_gridWidth)
            * static_cast<std::size_t>(_gridHeight),
        '.'
    );
    _textures.assign(
        static_cast<std::size_t>(_gridWidth)
            * static_cast<std::size_t>(_gridHeight),
        nullptr
    );
    _batches.clear();
}

void TileMap::updateVisibleVertices(const sf::View& view) {
    _batches.clear();
    if (_tiles.empty() || _textures.empty()
        || _gridWidth <= 0 || _gridHeight <= 0) {
        return;
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
            const std::size_t index = static_cast<std::size_t>(r)
                * static_cast<std::size_t>(_gridWidth)
                + static_cast<std::size_t>(c);
            const char tileCharacter = _tiles[index];
            const sf::Texture* texture = _textures[index];
            if (tileCharacter == '.' || texture == nullptr) {
                continue;
            }

            const float tu0 = 0.f;
            const float tv0 = 0.f;
            const float tu1 = static_cast<float>(texture->getSize().x);
            const float tv1 = static_cast<float>(texture->getSize().y);

            sf::VertexArray* batch = nullptr;
            for (Batch& candidate : _batches) {
                if (candidate.texture == texture) {
                    batch = &candidate.vertices;
                    break;
                }
            }
            if (batch == nullptr) {
                _batches.push_back(Batch{
                    texture,
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

}

char TileMap::getTile(int col, int row) const {
    if (row >= 0 && row < _gridHeight && col >= 0 && col < _gridWidth) {
        const std::size_t index = static_cast<std::size_t>(row)
            * static_cast<std::size_t>(_gridWidth)
            + static_cast<std::size_t>(col);
        return _tiles[index];
    }
    return '.';
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
