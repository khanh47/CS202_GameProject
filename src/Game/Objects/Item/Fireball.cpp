#include "Game/Objects/Item/Fireball.h"
#include "Physics/PhysicsUnits.h"
#include "ResourceManager.h"
#include "Animation/AnimationLibrary.h"

Fireball::Fireball() : GameObject(), Animatable() {
    sf::Texture& itemsTexture = ResourceManager::getInstance().getTexture("mario_and_items");
    configureVisuals(itemsTexture, "fireball");
}

Fireball::Fireball(sf::Texture& texture) : GameObject(), Animatable() {
    configureVisuals(texture, "fireball");
}

void Fireball::activate(sf::Vector2f spawnPos, bool facingRight) {
    _active = true;
    _facingRight = facingRight;
    _distanceTraveled = 0.0f;

    if (hasValidBody()) {
        const b2BodyId bodyId = _body->getId();
        b2Body_Enable(bodyId);
        b2Body_SetTransform(bodyId, PhysicsUnits::toMeters(spawnPos), b2Rot_identity);

        const float vx = _facingRight ? _moveSpeedMeters : -_moveSpeedMeters;
        // Initial downward angle throw trajectory (matches SMB NES fireball launch)
        b2Body_SetLinearVelocity(bodyId, {vx, 3.5f});
    }
}

void Fireball::deactivate() {
    _active = false;
    _distanceTraveled = 0.0f;

    if (hasValidBody()) {
        b2Body_Disable(_body->getId());
    }
}

void Fireball::triggerBounce() {
    if (hasValidBody() && _active) {
        const b2BodyId bodyId = _body->getId();
        b2Vec2 vel = b2Body_GetLinearVelocity(bodyId);
        // Only trigger bounce when falling or touching ground level (prevents double-bouncing)
        if (vel.y >= -1.0f) {
            vel.y = -_bounceImpulseMeters;
            b2Body_SetLinearVelocity(bodyId, vel);
        }
    }
}

void Fireball::updateSimulation(const float& fixedDt) {
    if (!_active || !hasValidBody()) {
        return;
    }

    const b2BodyId bodyId = _body->getId();
    b2Vec2 vel = b2Body_GetLinearVelocity(bodyId);
    
    // Maintain constant horizontal movement speed in direction facing
    vel.x = _facingRight ? _moveSpeedMeters : -_moveSpeedMeters;
    b2Body_SetLinearVelocity(bodyId, vel);

    // Track total horizontal distance traveled in SFML pixel space
    _distanceTraveled += std::abs(vel.x) * fixedDt * PhysicsUnits::pixelsPerMeter;

    playAnimation("spin");
}

void Fireball::onCreateBodyDef(b2BodyDef& def) {
    def.type = b2_dynamicBody;
    def.motionLocks.angularZ = true;
    def.gravityScale = 2.8f;
}

void Fireball::onCreateShapeDef(b2ShapeDef& def) {
    def.density = 0.5f;
    def.material.restitution = 0.0f;
    def.material.friction = 0.0f;
    def.enableContactEvents = true;

    // Filter bits: Category 0x0004 (Fireball), Mask 0x0001 (Collides ONLY with environment/blocks)
    // Fireballs pass freely through player (0x0002) and other fireballs (0x0004)
    def.filter.categoryBits = 0x0004;
    def.filter.maskBits = 0x0001;
}

void Fireball::onUpdateVisuals(float deltaTime) {
    if (!_active) {
        return;
    }
    updateVisualState(deltaTime, _hitboxPixels, !_facingRight);
}

void Fireball::onRenderVisual(sf::RenderTarget& target, const sf::Vector2f& position, float angleDegrees) {
    (void)angleDegrees;
    if (!_active) {
        return;
    }
    renderVisualState(target, position);
}
