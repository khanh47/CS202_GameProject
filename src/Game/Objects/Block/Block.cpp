#include "Game/Objects/Block/Block.h"
#include "ResourceManager.h"

Block::Block() : GameObject() {
    animatable = std::make_unique<Animatable>();
    animatable->configureVisuals(ResourceManager::getInstance().getTexture("tiles"));
}

Block::Block(sf::Texture &texture) : GameObject() {
    animatable = std::make_unique<Animatable>();
    animatable->configureVisuals(texture);
}

Block::~Block() {
}

void Block::onCreateShapeDef(b2ShapeDef& def) {
    def.density = 10000.0f;
    def.material.friction = 0.0f;
}

void Block::onUpdateVisuals(float deltaTime) {
    animatable->updateVisualState(deltaTime, _hitboxPixels);
}

void Block::onRenderVisual(sf::RenderTarget& target, const sf::Vector2f& position, float angleDegrees) {
    animatable->renderVisualState(target, position, angleDegrees);
}
