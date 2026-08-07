#include "Game/Objects/Block/CoinBlock.h"
#include "Game/Behaviours/Animatable.h"
#include "ResourceManager.h"

CoinBlock::CoinBlock() : GameObject() {
    addBehaviour<Animatable>();
    if (auto* animatable = getBehaviour<Animatable>()) {
        animatable->configureVisuals(
            ResourceManager::getInstance().getTexture("coin_block_spritesheet"),
            "coin_block"
        );
    }
}

CoinBlock::CoinBlock(sf::Texture &texture) : GameObject() {
    addBehaviour<Animatable>();
    if (auto* animatable = getBehaviour<Animatable>()) {
        animatable->configureVisuals(texture, "coin_block");
    }
}

CoinBlock::~CoinBlock() {
}

void CoinBlock::onCreateShapeDef(b2ShapeDef& def) {
    def.density = 10000.0f;
    def.material.friction = 0.0f;
}

void CoinBlock::onUpdateVisuals(float deltaTime) {
    if (auto* animatable = getBehaviour<Animatable>()) {
        animatable->updateVisualState(deltaTime, _hitboxPixels);
    }
}

void CoinBlock::onRenderVisual(sf::RenderTarget& target, const sf::Vector2f& position, float angleDegrees) {
    if (auto* animatable = getBehaviour<Animatable>()) {
        animatable->renderVisualState(target, position, angleDegrees);
    }
}
