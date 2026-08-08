#include "Game/Objects/Prop/Prop.h"
#include "Game/Behaviours/Animatable.h"

Prop::Prop() : GameObject() {
    addBehaviour<Animatable>();
}

Prop::Prop(sf::Texture &texture) : GameObject() {
    addBehaviour<Animatable>();
    if (auto* animatable = getBehaviour<Animatable>()) {
        animatable->configureVisuals(texture);
    }
}

Prop::~Prop() {
}

void Prop::onCreateBodyDef(b2BodyDef& def) {
    def.type = b2_staticBody;
}

void Prop::onCreateShapeDef(b2ShapeDef& def) {
    def.density = 0.0f;
    def.material.friction = 0.0f;

    // Category 0x0020 (Prop), Mask 0x0001 (Environment)
    // Only collides with the environment, never with other game objects.
    def.filter.categoryBits = 0x0020;
    def.filter.maskBits = 0x0001;
}

void Prop::onUpdateVisuals(float deltaTime) {
    if (auto* animatable = getBehaviour<Animatable>()) {
        animatable->updateVisualState(deltaTime, _hitboxPixels);
    }
}

void Prop::onRenderVisual(sf::RenderTarget& target, const sf::Vector2f& position, float angleDegrees) {
    if (auto* animatable = getBehaviour<Animatable>()) {
        animatable->renderVisualState(target, position, angleDegrees);
    }
}
