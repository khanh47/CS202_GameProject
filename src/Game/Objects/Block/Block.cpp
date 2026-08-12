#include "Game/Objects/Block/Block.h"
#include "Game/Objects/Player/Player.h"
#include "Game/Behaviours/Animatable.h"

Block::Block() : GameObject() {
    addBehaviour<Animatable>();
}

Block::Block(sf::Texture &texture) : Block() {
    if (auto* animatable = getBehaviour<Animatable>()) {
        animatable->configureVisuals(texture);
    }
}

void Block::onContact(GameObject& other, const b2ContactData& contactData, b2ShapeId ownShape) {
    isBumped(other, contactData, ownShape);
}

Block::~Block() {
}

void Block::onCreateShapeDef(b2ShapeDef& def) {
    def.density = 10000.0f;
    def.material.friction = 0.0f;
}

void Block::onUpdateVisuals(float deltaTime) {
    if (auto* animatable = getBehaviour<Animatable>()) {
        animatable->updateVisualState(deltaTime, _hitboxPixels);
    }
}

void Block::onRenderVisual(sf::RenderTarget& target, const sf::Vector2f& position, float angleDegrees) {
    if (auto* animatable = getBehaviour<Animatable>()) {
        animatable->renderVisualState(target, position, angleDegrees);
    }
}

bool Block::isBumped(GameObject& other, const b2ContactData& contactData, b2ShapeId ownShape) {
    if (auto* player = dynamic_cast<Player*>(&other)) {
        if (b2Shape_IsValid(ownShape) && contactData.manifold.pointCount > 0) {
            b2Vec2 normal = contactData.manifold.normal;
            if (!B2_ID_EQUALS(contactData.shapeIdA, ownShape)) {
                normal = {-normal.x, -normal.y};
            }

            // Detect Player hitting from below (upward contact normal or player below block)
            if (normal.y >= 0.3f || player->getPosition().y > getPosition().y) {
                if(auto* playerAnimatable = player->getBehaviour<Animatable>()) {
                    playerAnimatable->playAnimation("bump");
                    return true;
                }
            }
            else return false;
        }
    }
    return false;
}
