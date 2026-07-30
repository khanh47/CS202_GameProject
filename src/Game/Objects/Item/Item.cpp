#include "Game/Objects/Item/Item.h"

Item::Item() : GameObject() {
    animatable = std::make_unique<Animatable>();
}

Item::Item(sf::Texture& texture) : GameObject() {
    animatable = std::make_unique<Animatable>();
    animatable->configureVisuals(texture);
}

Item::~Item() {
}

void Item::onUpdateVisuals(float deltaTime) {
    animatable->updateVisualState(deltaTime, _hitboxPixels);
}

void Item::onRenderVisual(sf::RenderTarget& target, const sf::Vector2f& position, float angleDegrees) {
    animatable->renderVisualState(target, position, angleDegrees);
}
