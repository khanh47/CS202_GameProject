#include "Game/Objects/Pipe/Pipe.h"

#include "Physics/CollisionFilter.h"
#include "Physics/PhysicsUnits.h"
#include "box2d/box2d.h"

namespace {
/// Each spritesheet block is 16x16 pixels with 1px gaps between them.
constexpr int kSpriteTileSize = 16;
constexpr int kSpriteGap = 1;

/// World-space size of each rendered tile (matches one 64px cell).
/// A pipe is 2 tiles wide = 128px = 2 grid cells.
constexpr float kRenderTileSize = 64.0f;
}

Pipe::Pipe() : GameObject() {}

Pipe::Pipe(sf::Texture& texture, Orientation orientation, EndSide endSide,
           int bodyLength, bool isWarp, int warpID, int warpTarget)
    : GameObject(),
      _orientation(orientation),
      _endSide(endSide),
      _bodyLength(std::max(bodyLength, 0)),
      _isWarp(isWarp),
      _warpID(warpID),
      _warpTarget(warpTarget),
      _texture(&texture) {
    buildVertexArray(kRenderTileSize);
}

sf::Vector2f Pipe::computePipeSize(Orientation orientation, int bodyLength,
                                   float renderTileSize) {
    // Pipe is always 2 tiles in the cross-axis.
    // Along the main axis: 1 end-cap row + bodyLength body rows.
    const int mainAxisTiles = 1 + std::max(bodyLength, 0);
    if (orientation == Orientation::Vertical) {
        return {2.0f * renderTileSize, mainAxisTiles * renderTileSize};
    }
    return {mainAxisTiles * renderTileSize, 2.0f * renderTileSize};
}

void Pipe::onCreateBodyDef(b2BodyDef& def) {
    def.type = b2_staticBody;
}

void Pipe::onCreateShapeDef(b2ShapeDef& def) {
    def.density = 10000.0f;
    def.material.friction = 0.0f;
    def.filter.categoryBits = CollisionFilter::ENV;
    def.filter.maskBits = CollisionFilter::PLAYER | CollisionFilter::ENEMY |
                          CollisionFilter::FIREBALL | CollisionFilter::SHELL;
}

void Pipe::onRenderVisual(sf::RenderTarget& target, const sf::Vector2f& position,
                          float angleDegrees) {
    if (!_texture || _vertices.getVertexCount() == 0) {
        return;
    }

    // The vertex array is built relative to the pipe's center (0,0).
    // Translate to the body's world position.
    sf::RenderStates states;
    states.texture = _texture;
    states.transform.translate(position);
    target.draw(_vertices, states);
}

sf::IntRect Pipe::blockRect(int gridCol, int gridRow) {
    // gridCol and gridRow are 1-indexed.
    const int x = (gridCol - 1) * (kSpriteTileSize + kSpriteGap) + kSpriteGap;
    const int y = (gridRow - 1) * (kSpriteTileSize + kSpriteGap) + kSpriteGap;
    return {{x, y}, {kSpriteTileSize, kSpriteTileSize}};
}

void Pipe::appendQuad(sf::Vector2f worldPos, sf::Vector2f worldSize,
                      const sf::IntRect& texRect) {
    const float x0 = worldPos.x;
    const float y0 = worldPos.y;
    const float x1 = worldPos.x + worldSize.x;
    const float y1 = worldPos.y + worldSize.y;

    const auto u0 = static_cast<float>(texRect.position.x);
    const auto v0 = static_cast<float>(texRect.position.y);
    const auto u1 = static_cast<float>(texRect.position.x + texRect.size.x);
    const auto v1 = static_cast<float>(texRect.position.y + texRect.size.y);

    // Triangle 1
    _vertices.append({{x0, y0}, sf::Color::White, {u0, v0}});
    _vertices.append({{x1, y0}, sf::Color::White, {u1, v0}});
    _vertices.append({{x1, y1}, sf::Color::White, {u1, v1}});

    // Triangle 2
    _vertices.append({{x0, y0}, sf::Color::White, {u0, v0}});
    _vertices.append({{x1, y1}, sf::Color::White, {u1, v1}});
    _vertices.append({{x0, y1}, sf::Color::White, {u0, v1}});
}

void Pipe::buildVertexArray(float tileSize) {
    _vertices.clear();

    const sf::Vector2f pipeSize = computePipeSize(_orientation, _bodyLength, tileSize);
    // Offset so the vertex array is centered at (0,0) — matching Box2D body center.
    const float halfW = pipeSize.x * 0.5f;
    const float halfH = pipeSize.y * 0.5f;

    if (_orientation == Orientation::Vertical) {
        // Vertical pipe: 2 tiles wide, (1 + _bodyLength) tiles tall.
        // EndSide::Top  => cap row first, then body rows going down.
        // EndSide::Bottom => body rows first, then cap row at the bottom.

        const int totalRows = 1 + _bodyLength;

        for (int row = 0; row < totalRows; ++row) {
            for (int col = 0; col < 2; ++col) {
                const float worldX = -halfW + col * tileSize;
                const float worldY = -halfH + row * tileSize;

                sf::IntRect texRect;

                if (_endSide == EndSide::Top) {
                    if (row == 0) {
                        // Cap row: blocks (1,1) and (2,1)
                        texRect = blockRect(col + 1, 1);
                    } else {
                        // Body row: blocks (1,2) and (2,2)
                        texRect = blockRect(col + 1, 2);
                    }
                } else {
                    // EndSide::Bottom — reversed pipe
                    if (row == totalRows - 1) {
                        // Cap row at bottom: blocks (1,3) and (2,3)
                        texRect = blockRect(col + 1, 3);
                    } else {
                        // Body row: blocks (1,2) and (2,2)
                        texRect = blockRect(col + 1, 2);
                    }
                }

                appendQuad({worldX, worldY}, {tileSize, tileSize}, texRect);
            }
        }
    } else {
        // Horizontal pipe: 2 tiles tall, (1 + _bodyLength) tiles wide.
        // EndSide::Left  => cap column first, then body columns going right.
        // EndSide::Right => body columns first, then cap column at the right.

        const int totalCols = 1 + _bodyLength;

        for (int row = 0; row < 2; ++row) {
            for (int col = 0; col < totalCols; ++col) {
                const float worldX = -halfW + col * tileSize;
                const float worldY = -halfH + row * tileSize;

                sf::IntRect texRect;

                if (_endSide == EndSide::Left) {
                    if (col == 0) {
                        // Left cap: blocks (3,1)/(3,2) for upper/lower
                        texRect = blockRect(3, row + 1);
                    } else {
                        // Body: blocks (4,1)/(4,2)
                        texRect = blockRect(4, row + 1);
                    }
                } else {
                    // EndSide::Right
                    if (col == totalCols - 1) {
                        // Right cap: blocks (5,1)/(5,2)
                        texRect = blockRect(5, row + 1);
                    } else {
                        // Body: blocks (4,1)/(4,2)
                        texRect = blockRect(4, row + 1);
                    }
                }

                appendQuad({worldX, worldY}, {tileSize, tileSize}, texRect);
            }
        }
    }
}
