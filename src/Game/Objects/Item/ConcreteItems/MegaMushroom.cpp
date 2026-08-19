#include "Game/Objects/Item/ConcreteItems/MegaMushroom.h"

#include "Game/Objects/Player/Player.h"
#include "Game/Behaviours/Animatable.h"
#include "Game/GameSettings.h"
#include "Physics/CollisionFilter.h"
#include "box2d/box2d.h"

MegaMushroom::MegaMushroom() : Item() {
}

MegaMushroom::MegaMushroom(sf::Texture& texture) : Item() {
    if (auto* animatable = getBehaviour<Animatable>()) {
        animatable->configureVisuals(texture, "mega_mushroom");
    }
}

void MegaMushroom::onPickup(Player& player) {
    if (auto* world = player.getGameWorld()) {
        player.startTransformation(Player::TransformTarget::Mega, *world);
    } else {
        player.applyMegaState(Player::megaStateDurationSeconds);
    }
    destroy();
}

void MegaMushroom::onCreateBodyDef(b2BodyDef& def) {
    def.type = b2_dynamicBody;
    def.motionLocks.angularZ = true;
}

void MegaMushroom::onCreateShapeDef(b2ShapeDef& def) {
    def.isSensor = false;
    def.density = 0.5f;
    def.material.friction = 0.0f;
    def.material.restitution = 0.95f;
    def.filter.categoryBits = CollisionFilter::PICKUP;
    def.filter.maskBits = CollisionFilter::ENV | CollisionFilter::PLAYER;
}

void MegaMushroom::updateSimulation(const float& fixedDt) {
    Item::updateSimulation(fixedDt);
    if (isEmerging()) {
        return;
    }

    if (!hasValidBody()) {
        return;
    }

    const b2BodyId body = _body->getId();
    const b2Vec2 velocity = b2Body_GetLinearVelocity(body);
    if (_movingRight && velocity.x < -0.5f) {
        _movingRight = false;
    } else if (!_movingRight && velocity.x > 0.5f) {
        _movingRight = true;
    }

    const float direction = _movingRight ? 1.0f : -1.0f;
    const float horizontalSpeed =
        GameSettings::getInstance().gameMode == GameMode::Minigame
        ? 0.0f
        : _speed;
    b2Body_SetLinearVelocity(body, {horizontalSpeed * direction, velocity.y});
}
