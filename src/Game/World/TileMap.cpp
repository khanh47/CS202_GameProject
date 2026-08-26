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
    _tiles.assign(cellCount, TileInfo{});
    _overlayTiles.assign(cellCount, TileInfo{});
    _batches.clear();
    _brickFrame = 0;
    _brickFrameElapsed = 0.0f;
    _hasAnimatedTiles = false;
}

void TileMap::setTile(
    int col,
    int row,
    char tileCharacter,
    const sf::Texture* texture,
    sf::IntRect textureRect,
    TileAnimation animation
) {
    if (row >= 0 && row < _gridHeight && col >= 0 && col < _gridWidth) {
        const std::size_t index = static_cast<std::size_t>(row)
            * static_cast<std::size_t>(_gridWidth)
            + static_cast<std::size_t>(col);
        _tiles[index] = TileInfo{
            tileCharacter,
            texture,
            textureRect,
            animation
        };
        _hasAnimatedTiles = _hasAnimatedTiles
            || animation != TileAnimation::None;
    }
}

void TileMap::setOverlayTile(
    int col,
    int row,
    char tileCharacter,
    const sf::Texture* texture,
    sf::IntRect textureRect
) {
    if (row >= 0 && row < _gridHeight && col >= 0 && col < _gridWidth) {
        const std::size_t index = static_cast<std::size_t>(row)
            * static_cast<std::size_t>(_gridWidth)
            + static_cast<std::size_t>(col);
        _overlayTiles[index] = TileInfo{tileCharacter, texture, textureRect};
    }
}

void TileMap::clear() {
    const std::size_t cellCount = static_cast<std::size_t>(_gridWidth)
        * static_cast<std::size_t>(_gridHeight);
    _tiles.assign(cellCount, TileInfo{});
    _overlayTiles.assign(cellCount, TileInfo{});
    _batches.clear();
    _brickFrame = 0;
    _brickFrameElapsed = 0.0f;
    _hasAnimatedTiles = false;
}

void TileMap::update(float deltaTime) {
    if (!_hasAnimatedTiles || deltaTime <= 0.0f) {
        return;
    }

    constexpr float brickFrameDuration = 1.0f / 4.0f;
    constexpr std::size_t brickFrameCount = 4;

    _brickFrameElapsed += deltaTime;
    while (_brickFrameElapsed >= brickFrameDuration) {
        _brickFrameElapsed -= brickFrameDuration;
        _brickFrame = (_brickFrame + 1) % brickFrameCount;
    }
}

void TileMap::updateVisibleVertices(const sf::View& view) {
    _batches.clear();
    if (_tiles.empty()
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

    // Helper lambda to append quads for a tile vector
    const auto appendTiles = [&](const std::vector<TileInfo>& tileVector) {
        for (int r = startRow; r <= endRow; ++r) {
            for (int c = startCol; c <= endCol; ++c) {
                const std::size_t index = static_cast<std::size_t>(r)
                    * static_cast<std::size_t>(_gridWidth)
                    + static_cast<std::size_t>(c);
                const TileInfo& tile = tileVector[index];
                if (tile.character == '.' || tile.texture == nullptr) {
                    continue;
                }

                const sf::Texture* texture = tile.texture;
                sf::IntRect textureRect = tile.textureRect;
                if (tile.animation == TileAnimation::Brick) {
                    textureRect = sf::IntRect(
                        {
                            static_cast<int>(_brickFrame * 72),
                            0
                        },
                        {64, 64}
                    );
                } else if (textureRect.size.x <= 0 || textureRect.size.y <= 0) {
                    textureRect = sf::IntRect(
                        {0, 0},
                        {
                            static_cast<int>(texture->getSize().x),
                            static_cast<int>(texture->getSize().y)
                        }
                    );
                }

                const float tu0 = static_cast<float>(textureRect.position.x);
                const float tv0 = static_cast<float>(textureRect.position.y);
                const float tu1 = static_cast<float>(
                    textureRect.position.x + textureRect.size.x
                );
                const float tv1 = static_cast<float>(
                    textureRect.position.y + textureRect.size.y
                );

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
                batch->append(sf::Vertex({left, top}, sf::Color::White, {tu0, tv0}));
                batch->append(sf::Vertex({right, top}, sf::Color::White, {tu1, tv0}));
                batch->append(sf::Vertex({right, bottom}, sf::Color::White, {tu1, tv1}));

                batch->append(sf::Vertex({left, top}, sf::Color::White, {tu0, tv0}));
                batch->append(sf::Vertex({right, bottom}, sf::Color::White, {tu1, tv1}));
                batch->append(sf::Vertex({left, bottom}, sf::Color::White, {tu0, tv1}));
            }
        }
    };

    // Pass 1: Base tiles (Back Layer)
    appendTiles(_tiles);

    // Pass 2: Overlay tiles (Outer / Front Layer)
    appendTiles(_overlayTiles);
}

char TileMap::getTile(int col, int row) const {
    if (row >= 0 && row < _gridHeight && col >= 0 && col < _gridWidth) {
        const std::size_t index = static_cast<std::size_t>(row)
            * static_cast<std::size_t>(_gridWidth)
            + static_cast<std::size_t>(col);
        return _tiles[index].character;
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
