#include "Game/Objects/GameObject.h"
#include "Physics/PhysicsUnits.h"
#include <SFML/System/Vector2.hpp>
#include <memory>
#include <stdexcept>
#include "box2d/box2d.h"

namespace {
constexpr bool drawFallbackCollisionRect = true;
}

GameObject::GameObject() = default;

void GameObject::updateSimulation(const float &fixedDt) {

}

sf::Vector2f GameObject::getPosition() const {
    return getBodyPositionPixels();
}

sf::Vector2f GameObject::getVelocity() const {
    // Converts Box2D linear velocity (MKS meters/sec) to SFML pixel coordinates (pixels/sec)
    // for camera tracking and motion anticipation.
    if (hasValidBody()) {
        const b2Vec2 velocityMeters = b2Body_GetLinearVelocity(_body->getId());
        return PhysicsUnits::toPixels(velocityMeters);
    }
    return {0.0f, 0.0f};
}

void GameObject::updateVisuals(float deltaTime) {
    onUpdateVisuals(deltaTime);
}

void GameObject::render(sf::RenderTarget &target) { // DEFINITELY NEEDS TO BE REFRACTORED
    if (!hasValidBody()) return;

    const sf::Vector2f position = getBodyPositionPixels();
    const float angleDegrees = getBodyAngleDegrees();

    if (drawFallbackCollisionRect) {
        drawFallbackRect(target);
    }

    onRenderVisual(target, position, angleDegrees);
}

void GameObject::spawn(const PhysicsWorld &physicsWorld, sf::Vector2f spawnPixels, sf::Vector2f hitboxPixels) {
    if (!physicsWorld.isValid())
        throw std::runtime_error("Invalid World!");
    if(_body && _body->isValid())
        throw std::runtime_error("The player has already been spawned!");

    createBody(physicsWorld, spawnPixels);
    createHitbox(hitboxPixels);
    updateVisuals(0.f);
}

void GameObject::onCreateBodyDef(b2BodyDef& def) {
    (void)def;
}

void GameObject::onCreateShapeDef(b2ShapeDef& def) {
    (void)def;
}

void GameObject::onUpdateVisuals(float deltaTime) {
    (void)deltaTime;
}

void GameObject::onRenderVisual(sf::RenderTarget& target, const sf::Vector2f& position, float angleDegrees) {
    (void)target;
    (void)position;
    (void)angleDegrees;
}

bool GameObject::hasValidBody() const {
    return _body && _body->isValid();
}

sf::Vector2f GameObject::getBodyPositionPixels() const {
    if (hasValidBody()) {
        return PhysicsUnits::toPixels(b2Body_GetPosition(_body->getId()));
    }
    return {0.f, 0.f};
}

float GameObject::getBodyAngleDegrees() const {
    if (!hasValidBody()) {
        return 0.f;
    }

    const b2Rot rotation = b2Body_GetRotation(_body->getId());
    return b2Rot_GetAngle(rotation) * (180.f / 3.14159265f);
}

void GameObject::createBody(const PhysicsWorld &physicsWorld, sf::Vector2f spawnPixels) {
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.position = PhysicsUnits::toMeters(spawnPixels);
    bodyDef.userData = this;
    onCreateBodyDef(bodyDef);

    _body = std::make_shared<PhysicsBody>(physicsWorld, bodyDef);
}

void GameObject::createHitbox(sf::Vector2f hitboxPixels) {
    _hitboxPixels = hitboxPixels;

    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.enableContactEvents = true;
    shapeDef.userData = new std::weak_ptr<GameObject>(shared_from_this());
    onCreateShapeDef(shapeDef);

    b2Polygon box = b2MakeBox(
        PhysicsUnits::toMeters(hitboxPixels.x * 0.5f),
        PhysicsUnits::toMeters(hitboxPixels.y * 0.5f)
    );
    
    b2ShapeId hitbox = b2CreatePolygonShape(_body->getId(), &shapeDef, &box);
    _body->setHibox(hitbox);
}

void GameObject::drawFallbackRect(sf::RenderTarget& target) const {
    if (!_body || !_body->isValid()) {
        return;
    }

    const b2Vec2 position = b2Body_GetPosition(_body->getId());
    const b2Rot rotation = b2Body_GetRotation(_body->getId());
    const float angleDegrees = b2Rot_GetAngle(rotation) * (180.f / 3.14159265f);
    const sf::Vector2f sizePixels = _hitboxPixels;

    sf::RectangleShape fallbackRect(sizePixels);
    fallbackRect.setOrigin({sizePixels.x / 2.f, sizePixels.y / 2.f});
    fallbackRect.setPosition(PhysicsUnits::toPixels(position));
    fallbackRect.setRotation(sf::degrees(angleDegrees));
    fallbackRect.setFillColor(sf::Color::Magenta);
    fallbackRect.setOutlineThickness(1.f);
    fallbackRect.setOutlineColor(sf::Color::Black);

    target.draw(fallbackRect);
}
