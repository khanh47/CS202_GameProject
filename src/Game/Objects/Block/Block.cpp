#include "Game/Objects/Block/Block.h"
#include "ResourceManager.h"

Block::Block() : GameObject(), Animatable() {
    configureVisuals(ResourceManager::getInstance().getTexture("tiles"));
}

Block::Block(sf::Texture &texture) : GameObject(), Animatable() {
    configureVisuals(texture);
}

Block::~Block() {
}

void Block::onCreateShapeDef(b2ShapeDef& def) {
    def.density = 1.0f;
    def.material.friction = 0.0f;
}

void Block::onUpdateVisuals(float deltaTime) {
    updateVisualState(deltaTime, _hitboxPixels);
}

void Block::onRenderVisual(sf::RenderTarget& target, const sf::Vector2f& position, float angleDegrees) {
    renderVisualState(target, position, angleDegrees);
}
