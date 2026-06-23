#include <stdexcept>

#include "Game/Objects/Blocks/Block.h"
#include "Physics/PhysicsUnits.h"
#include "Physics/PhysicsWorld.h"
#include "ResourceManager.h"

Block::Block() : GameObject() {
    _sprite = sf::Sprite(ResourceManager::getInstance().getTexture("brick")); // temporary, may implement inheritance later

    if (_sprite.has_value()) {
        sf::FloatRect bounds = _sprite.value().getLocalBounds();
        _sprite->setOrigin({bounds.size.x / 2.f, bounds.size.y / 2.f});
    }
    
}

Block::Block(sf::Texture &texture) : GameObject() {
    _sprite = sf::Sprite(texture);
}

Block::~Block() {
}


void Block::spawn(PhysicsWorld* physicsWorld, sf::Vector2f spawnPixels, sf::Vector2f hitboxPixels) {
    if (!physicsWorld || !physicsWorld->isValid())
        throw std::runtime_error("Invalid World!");
    if(_body && _body->isValid())
        throw std::runtime_error("The player has already been spawned!");

    createBody(physicsWorld, spawnPixels);
    createHitbox(hitboxPixels);
}

void Block::createBody(PhysicsWorld* physicsWorld, sf::Vector2f spawnPixels) {
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.position = PhysicsUnits::toMeters(spawnPixels);
    bodyDef.userData = this;

    _body = std::make_shared<PhysicsBody>(physicsWorld->getId(), bodyDef);
}

void Block::createHitbox(sf::Vector2f hitboxPixels) {
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = 1.0f;
    shapeDef.material.friction = 0.0f;
    shapeDef.enableContactEvents = true;
    shapeDef.userData = this;

    b2Polygon box = b2MakeBox(
        PhysicsUnits::toMeters(hitboxPixels.x),
        PhysicsUnits::toMeters(hitboxPixels.y)
    );

    b2ShapeId hitbox = b2CreatePolygonShape(_body->getId(), &shapeDef, &box);
    _body->setHibox(hitbox);
}