#include "Game/Objects/Item/Coin.h"
#include "Physics/PhysicsUnits.h"

Coin::Coin()
    : GameObject(), Animatable(), Damageable(20) {
}

Coin::Coin(sf::Texture& texture)
    : GameObject(), Animatable(), Damageable(20) {
    configureVisuals(texture, "Coin");
}

Coin::~Coin() = default;

void Coin::onCreateBodyDef(b2BodyDef& def) {
    def.type = b2_dynamicBody;
    startMoveLeft();
}

void Coin::onCreateShapeDef(b2ShapeDef& def) {
    def.density = 1.0f;
}

void Coin::updateSimulation(const float& fixedDt) {
    if (!hasValidBody()) return;

    b2BodyId body = _body->getId();
    float speed = 0.0f;
    float dir = 0.0f;
    b2Vec2 vel = b2Body_GetLinearVelocity(body);
    b2Body_SetLinearVelocity(body, { speed * dir, vel.y });
}

void Coin::onUpdateVisuals(float deltaTime) {
    updateVisualState(deltaTime, _hitboxPixels, isFacingLeft());
}

void Coin::onRenderVisual(sf::RenderTarget& target,
                            const sf::Vector2f& position,
                            float angleDegrees) {
    renderVisualState(target, position, angleDegrees);
}