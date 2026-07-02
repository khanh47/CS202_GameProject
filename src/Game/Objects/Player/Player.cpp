#include <stdexcept>

#include "Game/Objects/Player/Player.h"
#include "Physics/PhysicsUnits.h"
#include "Physics/PhysicsWorld.h"
#include "ResourceManager.h"

Player::Player() : GameObject(), Damageable(100) {
    // _sprite = sf::Sprite(ResourceManager::getInstance().getTexture("brick"));

    if (_sprite.has_value()) {
        sf::FloatRect bounds = _sprite.value().getLocalBounds();
        _sprite->setOrigin({bounds.size.x / 2.f, bounds.size.y / 2.f});
    }
    
}

Player::Player(sf::Texture &texture) : GameObject(), Damageable(100) {
    _sprite = sf::Sprite(texture);
}

Player::~Player() {
}


void Player::spawn(const PhysicsWorld &physicsWorld, sf::Vector2f spawnPixels, sf::Vector2f hitboxPixels) {
    if (!physicsWorld.isValid())
        throw std::runtime_error("Invalid World!");
    if(_body && _body->isValid())
        throw std::runtime_error("The player has already been spawned!");

    createBody(physicsWorld, spawnPixels);
    createHitbox(hitboxPixels);
}

void Player::createBody(const PhysicsWorld &physicsWorld, sf::Vector2f spawnPixels) {
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = PhysicsUnits::toMeters(spawnPixels);
    bodyDef.userData = this;

    _body = std::make_shared<PhysicsBody>(physicsWorld, bodyDef);
}

void Player::createHitbox(sf::Vector2f hitboxPixels) {
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = 1.0f;
    shapeDef.material.friction = 0.0f;
    shapeDef.enableContactEvents = true;
    shapeDef.userData = this;

    b2Polygon box = b2MakeBox(
        PhysicsUnits::toMeters(hitboxPixels.x / 2.0f),
        PhysicsUnits::toMeters(hitboxPixels.y / 2.0f)
    );

    b2ShapeId hitbox = b2CreatePolygonShape(_body->getId(), &shapeDef, &box);
    _body->setHibox(hitbox);
}