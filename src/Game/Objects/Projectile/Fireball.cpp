#include "Game/Objects/Projectile/Fireball.h"
#include "Game/Behaviours/Animatable.h"
#include "Physics/PhysicsUnits.h"
#include "ResourceManager.h"
#include "Game/Objects/Block/Block.h"
#include "Game/Objects/Player/Player.h"
#include "Game/Objects/Enemy/Enemy.h"
#include "Game/World/GameWorld.h"
#include "Game/ScoreManager.h"
#include "Game/GameSettings.h"

Fireball::Fireball() : GameObject() {
    addBehaviour<Animatable>();
    sf::Texture& itemsTexture = ResourceManager::getInstance().getTexture("mario_and_items");
    if (auto* animatable = getBehaviour<Animatable>()) {
        animatable->configureVisuals(itemsTexture, "fireball");
    }
}

Fireball::Fireball(sf::Texture& texture) : GameObject() {
    addBehaviour<Animatable>();
    if (auto* animatable = getBehaviour<Animatable>()) {
        animatable->configureVisuals(texture, "fireball");
    }
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

    if (auto* animatable = getBehaviour<Animatable>()) {
        animatable->playAnimation("spin");
    }
}

void Fireball::onContact(GameObject& other, const b2ContactData&, b2ShapeId) {
    if (auto* block = dynamic_cast<Block*>(&other)) {
        const sf::Vector2f difference = getPosition() - block->getPosition();
        if (difference.y < -16.0f) {
            triggerBounce();
        } else {
            deactivate();
        }
        return;
    }

    if (auto* enemy = dynamic_cast<Enemy*>(&other)) {
        if (_world && _world->getScoreManager()) {
            // Triggers 100 -> 200 -> 400 -> 800 -> 1000 -> 2000 -> 4000 -> 8000 -> 1UP
            _world->getScoreManager()->handleEvent(ScoreEventType::EnemyStomped, enemy->getPosition());
        }

        other.destroy();
        deactivate();
    }
    if (auto* player = dynamic_cast<Player*>(&other)) {
        other.destroy();
        deactivate();
    }
}

void Fireball::onCreateBodyDef(b2BodyDef& def) {
    def.type = b2_dynamicBody;
    def.motionLocks.angularZ = true;
    def.gravityScale = 2.8f;
    def.isBullet = true;
}

void Fireball::onCreateShapeDef(b2ShapeDef& def) {
    def.density = 0.5f;
    def.material.restitution = 0.0f;
    def.material.friction = 0.0f;
    def.enableContactEvents = true;

    // Filter bits: Category 0x0004 (Fireball), Mask 0x0001 (Collides ONLY with environment/blocks)
    // Fireballs pass freely through player (0x0002) and other fireballs (0x0004)
    def.filter.categoryBits = 0x0004;
    def.filter.maskBits = 0x0001 | 0x0008;
    if (GameSettings::getInstance().gameMode == GameMode::Minigame) {
        def.filter.maskBits |= 0x0002;
    }
}

void Fireball::onUpdateVisuals(float deltaTime) {
    if (!_active) {
        return;
    }
    if (auto* animatable = getBehaviour<Animatable>()) {
        animatable->updateVisualState(deltaTime, _hitboxPixels, !_facingRight);
    }
}

void Fireball::onRenderVisual(sf::RenderTarget& target, const sf::Vector2f& position, float angleDegrees) {
    (void)angleDegrees;
    if (!_active) {
        return;
    }
    if (auto* animatable = getBehaviour<Animatable>()) {
        animatable->renderVisualState(target, position);
    }
}
