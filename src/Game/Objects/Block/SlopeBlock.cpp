#include "Game/Objects/Block/SlopeBlock.h"
#include "Game/Behaviours/Animatable.h"
#include "Physics/PhysicsUnits.h"

SlopeBlock::SlopeBlock() : Block() {}

SlopeBlock::SlopeBlock(sf::Texture& texture, SlopeType slopeType)
    : Block(texture), _slopeType(slopeType) {
    configureSlopeVisuals(texture, slopeType);
}

SlopeBlock::SlopeType SlopeBlock::parseSlopeType(const std::string& str) {
    if (str == "up_right_top" || str == "26") return SlopeType::UpRightTop;
    if (str == "down_right_top" || str == "27") return SlopeType::DownRightTop;
    if (str == "down_right_bottom" || str == "28") return SlopeType::DownRightBottom;
    return SlopeType::UpRightBottom;
}

void SlopeBlock::setSlopeType(SlopeType type) {
    _slopeType = type;
}

void SlopeBlock::configureSlopeVisuals(sf::Texture& texture, SlopeType slopeType) {
    _slopeType = slopeType;
    if (auto* animatable = getBehaviour<Animatable>()) {
        animatable->configureVisuals(texture);
        sf::IntRect rect;
        switch (_slopeType) {
            case SlopeType::UpRightBottom:
            case SlopeType::UpRightTop:
                rect = sf::IntRect({0, 208}, {16, 16});
                break;
            case SlopeType::DownRightTop:
            case SlopeType::DownRightBottom:
                rect = sf::IntRect({32, 208}, {16, 16});
                break;
        }
        animatable->setTextureRect(rect);
    }
}

void SlopeBlock::onCreateShapeDef(b2ShapeDef& def) {
    Block::onCreateShapeDef(def);
    def.material.friction = 0.0f;
}

b2Polygon SlopeBlock::makeHitbox(sf::Vector2f hitboxPixels) const {
    const float hx = PhysicsUnits::toMeters(hitboxPixels.x * 0.5f);
    const float hy = PhysicsUnits::toMeters(hitboxPixels.y * 0.5f);

    b2Vec2 vertices[3];

    if (_slopeType == SlopeType::DownRightTop || _slopeType == SlopeType::DownRightBottom) {
        // Downward slope (top-left to bottom-right)
        vertices[0] = {-hx, -hy};
        vertices[1] = {-hx, hy};
        vertices[2] = {hx, hy};
    } else {
        // Upward slope (bottom-left to top-right)
        vertices[0] = {-hx, hy};
        vertices[1] = {hx, hy};
        vertices[2] = {hx, -hy};
    }

    const b2Hull hull = b2ComputeHull(vertices, 3);
    return b2MakePolygon(&hull, 0.0f);
}
