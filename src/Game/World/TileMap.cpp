#include "Game/World/TileMap.h"
#include <cmath>
#include <algorithm>

TileMap::TileMap() {
    _vertices.setPrimitiveType(sf::PrimitiveType::Triangles);
}

void TileMap::initialize(int gridWidth, int gridHeight, float cellSize) {
    _gridWidth = gridWidth;
    _gridHeight = gridHeight;
    _cellSize = cellSize;

    _tiles.assign(_gridHeight, std::vector<TileInfo>(_gridWidth, TileInfo{0, sf::IntRect()}));
}

void TileMap::setTexture(const sf::Texture* texture) {
    _texture = texture;
}

void TileMap::setTile(int col, int row, int tileId, const sf::IntRect& texRect) {
    if (row >= 0 && row < _gridHeight && col >= 0 && col < _gridWidth) {
        _tiles[row][col] = TileInfo{tileId, texRect};
    }
}

void TileMap::clear() {
    _tiles.assign(_gridHeight, std::vector<TileInfo>(_gridWidth, TileInfo{0, sf::IntRect()}));
    _vertices.clear();
}

void TileMap::updateVisibleVertices(const sf::View& view) {
    _vertices.clear();
    if (_tiles.empty() || _gridWidth <= 0 || _gridHeight <= 0) {
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
            const TileInfo& tile = _tiles[r][c];
            if (tile.tileId <= 0) {
                continue;
            }

            const float left = c * _cellSize;
            const float top = r * _cellSize;
            const float right = left + _cellSize;
            const float bottom = top + _cellSize;

            float tu0 = 0.f, tv0 = 0.f, tu1 = 0.f, tv1 = 0.f;
            if (tile.texRect.size.x > 0 && tile.texRect.size.y > 0) {
                tu0 = static_cast<float>(tile.texRect.position.x);
                tv0 = static_cast<float>(tile.texRect.position.y);
                tu1 = tu0 + static_cast<float>(tile.texRect.size.x);
                tv1 = tv0 + static_cast<float>(tile.texRect.size.y);
            } else if (_texture != nullptr) {
                tu0 = 0.f;
                tv0 = 0.f;
                tu1 = static_cast<float>(_texture->getSize().x);
                tv1 = static_cast<float>(_texture->getSize().y);
            }

            // Quad construction using 2 triangles (6 vertices)
            // Triangle 1: Top-Left, Top-Right, Bottom-Right
            _vertices.append(sf::Vertex({left, top}, sf::Color::White, {tu0, tv0}));
            _vertices.append(sf::Vertex({right, top}, sf::Color::White, {tu1, tv0}));
            _vertices.append(sf::Vertex({right, bottom}, sf::Color::White, {tu1, tv1}));

            // Triangle 2: Top-Left, Bottom-Right, Bottom-Left
            _vertices.append(sf::Vertex({left, top}, sf::Color::White, {tu0, tv0}));
            _vertices.append(sf::Vertex({right, bottom}, sf::Color::White, {tu1, tv1}));
            _vertices.append(sf::Vertex({left, bottom}, sf::Color::White, {tu0, tv1}));
        }
    }
}

int TileMap::getTileId(int col, int row) const {
    if (row >= 0 && row < _gridHeight && col >= 0 && col < _gridWidth) {
        return _tiles[row][col].tileId;
    }
    return 0;
}

void TileMap::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    if (_texture != nullptr) {
        states.texture = _texture;
    }
    target.draw(_vertices, states);
}
