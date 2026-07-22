#include "Game/Objects/Item/Item.h"

Item::Item() : GameObject(), Animatable() {
}

Item::Item(sf::Texture& texture) : GameObject(), Animatable() {
    configureVisuals(texture);
}

Item::~Item() {
}

void Item::onUpdateVisuals(float deltaTime) {
    updateVisualState(deltaTime, _hitboxPixels);
}

void Item::onRenderVisual(sf::RenderTarget& target, const sf::Vector2f& position, float angleDegrees) {
    renderVisualState(target, position, angleDegrees);
}
