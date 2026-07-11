#include "Game/Objects/GameObject.h"
#include "Animation/Animator.h"
#include "Physics/PhysicsUnits.h"
#include "box2d/math_functions.h"
#include <SFML/System/Vector2.hpp>
#include <memory>
#include <stdexcept>

GameObject::GameObject() {
    _animator = Animator();
}

void GameObject::updateSimulation(const float &fixedDt) {

}

void GameObject::updateVisuals(float deltaTime) {
    const bool frameChanged = _animator.update(deltaTime);

    if (_sprite && _animator.hasActiveAnimation()) {
        const sf::IntRect currentRect = _animator.getCurrentTextureRect();
        if (frameChanged || _sprite->getTextureRect() != currentRect) {
            _sprite->setTextureRect(currentRect);
            updateSpriteLayout();
        }
    }
}

void GameObject::render(sf::RenderTarget &target) { // DEFINITELY NEEDS TO BE REFRACTORED
    if (!_body || !_body->isValid()) return;

    b2Vec2 position = b2Body_GetPosition(_body->getId());
    b2Rot rotation = b2Body_GetRotation(_body->getId());
    float angleDegrees = b2Rot_GetAngle(rotation) * (180.f / 3.14159265f);

    drawFallbackRect(target); // for debugging
    if (!_sprite.has_value()) {
        return;
    }

    sf::Sprite& sprite = *_sprite;
    sprite.setPosition(PhysicsUnits::toPixels(position));
    sprite.setRotation(sf::degrees(angleDegrees));
    target.draw(sprite);
}

void GameObject::spawn(const PhysicsWorld &physicsWorld, sf::Vector2f spawnPixels, sf::Vector2f hitboxPixels) {
    if (!physicsWorld.isValid())
        throw std::runtime_error("Invalid World!");
    if(_body && _body->isValid())
        throw std::runtime_error("The player has already been spawned!");

    createBody(physicsWorld, spawnPixels);
    createHitbox(hitboxPixels);
    updateSpriteLayout();
}

void GameObject::onCreateBodyDef(b2BodyDef& def) {
    (void)def;
}

void GameObject::onCreateShapeDef(b2ShapeDef& def) {
    (void)def;
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
    shapeDef.userData = this;
    onCreateShapeDef(shapeDef);

    b2Polygon box = b2MakeBox(
        PhysicsUnits::toMeters(hitboxPixels.x * 0.5f),
        PhysicsUnits::toMeters(hitboxPixels.y * 0.5f)
    );
    
    b2ShapeId hitbox = b2CreatePolygonShape(_body->getId(), &shapeDef, &box);
    _body->setHibox(hitbox);
}

void GameObject::updateSpriteLayout() {
    if (!_sprite || _hitboxPixels.x <= 0.f || _hitboxPixels.y <= 0.f) {
        return;
    }

    const sf::Vector2i frameSize = _sprite->getTextureRect().size;
    if (frameSize.x <= 0 || frameSize.y <= 0) {
        return;
    }

    _sprite->setOrigin({frameSize.x / 2.f, frameSize.y / 2.f});

    // if (_animator.hasActiveAnimation()) {
        _sprite->setScale({_hitboxPixels.x / static_cast<float>(frameSize.x),
                           _hitboxPixels.y / static_cast<float>(frameSize.y)});
    // }
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
