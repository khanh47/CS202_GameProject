#include "Game/Objects/Item/Item.h"
#include "Game/Behaviours/Animatable.h"

Item::Item() : GameObject() {
    addBehaviour<Animatable>();
}

Item::Item(sf::Texture& texture) : GameObject() {
    addBehaviour<Animatable>();
    if (auto* animatable = getBehaviour<Animatable>()) {
        animatable->configureVisuals(texture);
    }
}

Item::~Item() {
}

void Item::onUpdateVisuals(float deltaTime) {
    if (auto* animatable = getBehaviour<Animatable>()) {
        animatable->updateVisualState(deltaTime, _hitboxPixels);
    }
}

void Item::onRenderVisual(sf::RenderTarget& target, const sf::Vector2f& position, float angleDegrees) {
    if (auto* animatable = getBehaviour<Animatable>()) {
        animatable->renderVisualState(target, position, angleDegrees);
    }
}
