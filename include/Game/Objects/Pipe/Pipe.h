#pragma once

#include <SFML/Graphics.hpp>
#include <string>

#include "Game/Objects/GameObject.h"

/// Multi-tile pipe object assembled from a 16x16-block spritesheet.
/// Pipes are static environment objects that can be vertical or horizontal,
/// with a decorative end cap on one side and repeating body segments.
class Pipe : public GameObject {
public:
    enum class Orientation { Vertical, Horizontal };

    /// Which end has the decorative cap (opening).
    /// A vertical pipe with EndSide::Top has the cap at the top,
    /// body extends downward. One end only — never both caps.
    enum class EndSide {
        Top,
        Bottom,
        Left,
        Right
    };

    Pipe();
    Pipe(sf::Texture& texture, Orientation orientation, EndSide endSide,
         int bodyLength, bool isWarp);
    ~Pipe() override = default;

    Orientation getOrientation() const { return _orientation; }
    EndSide getEndSide() const { return _endSide; }
    int getBodyLength() const { return _bodyLength; }
    bool isWarp() const { return _isWarp; }

    /// Computes the total pixel size of this pipe based on orientation and body length.
    /// Each tile is rendered at renderTileSize (default 32px), and the pipe is
    /// 2 tiles wide in the cross-axis direction.
    static sf::Vector2f computePipeSize(Orientation orientation, int bodyLength,
                                        float renderTileSize = 32.0f);

protected:
    void onCreateBodyDef(b2BodyDef& def) override;
    void onCreateShapeDef(b2ShapeDef& def) override;
    void onRenderVisual(sf::RenderTarget& target, const sf::Vector2f& position,
                        float angleDegrees) override;

private:
    /// Builds the vertex array from the spritesheet tile coordinates.
    void buildVertexArray(float renderTileSize);

    /// Returns the texture rect for a spritesheet block at (gridCol, gridRow) (1-indexed).
    /// Each block is 16x16 with 1px gaps.
    static sf::IntRect blockRect(int gridCol, int gridRow);

    /// Appends 6 vertices (2 triangles) for one tile quad.
    void appendQuad(sf::Vector2f worldPos, sf::Vector2f worldSize,
                    const sf::IntRect& texRect);

    Orientation _orientation = Orientation::Vertical;
    EndSide _endSide = EndSide::Top;
    int _bodyLength = 1;
    bool _isWarp = false;
    sf::Texture* _texture = nullptr;
    sf::VertexArray _vertices{sf::PrimitiveType::Triangles};
};
