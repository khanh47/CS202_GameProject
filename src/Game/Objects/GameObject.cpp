#include "Game/Objects/GameObject.h"
#include "Physics/PhysicsUnits.h"
#include "box2d/math_functions.h"
#include <SFML/System/Vector2.hpp>

void GameObject::updateSimulation(const float &fixedDt) {

}

void GameObject::updateVisuals(float deltaTime) {
    if (!_animator.hasActiveAnimation()) {
        return;
    }

    while (deltaTime >= _animator.getCurrentFrameDuration()) {
        float frameDuration = _animator.getCurrentFrameDuration();
        if (frameDuration <= 0.f) {
            break;
        }

        deltaTime -= frameDuration;
        _animator.step();

        if (_sprite) {
            _sprite->setTextureRect(_animator.getCurrentTextureRect());
        }
    }
}

void GameObject::render(sf::RenderTarget &target) { // DEFINITELY NEEDS TO BE REFRACTORED
    if(!_body->isValid()) return;

    b2Vec2 position = b2Body_GetPosition(_body->getId());
    b2Vec2 drawPos = position;
    b2Rot rotation = b2Body_GetRotation(_body->getId());
    float angleDegrees = b2Rot_GetAngle(rotation) * (180.f / 3.14159265f);

    b2ShapeId shape = _body->getHitbox();
    b2ShapeType type = b2Shape_GetType(shape);


    //temp
    // rect
        if (type == b2_polygonShape) {
            


        } 
        // circle
        else if (type == b2_circleShape) {
            b2Circle circle = b2Shape_GetCircle(shape);
            float radiusPixels = PhysicsUnits::toPixels(circle.radius);

            sf::CircleShape fallbackCircle(radiusPixels);
            fallbackCircle.setOrigin({radiusPixels, radiusPixels});
            fallbackCircle.setPosition(PhysicsUnits::toPixels(position));
            fallbackCircle.setFillColor(sf::Color::Magenta);

            target.draw(fallbackCircle);
        }
    // end temp

    if (_sprite.has_value()) {
        sf::Sprite& sprite = *_sprite;
        
        if (type == b2_polygonShape) {
            b2Polygon polygon = b2Shape_GetPolygon(shape);
            float widthMeters = std::abs(polygon.vertices[0].x - polygon.vertices[2].x);
            float heightMeters = std::abs(polygon.vertices[0].y - polygon.vertices[2].y);
            sf::Vector2f sizePixels = PhysicsUnits::toPixels({widthMeters, heightMeters});
            
            sf::Vector2i texSize = sprite.getTextureRect().size; 
            float ratio = (sizePixels.x / texSize.x + sizePixels.y / texSize.y) / 2;
            sprite.setScale({sizePixels.x / texSize.x, sizePixels.y / texSize.y});

            drawPos.x += PhysicsUnits::toMeters(sizePixels.x - texSize.x) / 4;

            //rect
            sf::RectangleShape fallbackRect(sizePixels);
            fallbackRect.setOrigin({sizePixels.x / 2.f, sizePixels.y / 2.f});
            fallbackRect.setPosition(PhysicsUnits::toPixels(position));
            fallbackRect.setRotation(sf::degrees(angleDegrees));
            fallbackRect.setFillColor(sf::Color::Magenta);

            target.draw(fallbackRect);
        } 
        else if (type == b2_circleShape) {
            b2Circle circle = b2Shape_GetCircle(shape);
            float radiusPixels = PhysicsUnits::toPixels(circle.radius);
            float diameterPixels = radiusPixels * 2.f;
            
            sf::Vector2u texSize = sprite.getTexture().getSize();
            sprite.setScale({diameterPixels / texSize.x, diameterPixels / texSize.y});
        }

        sprite.setOrigin({sprite.getLocalBounds().size.x / 2.f, sprite.getLocalBounds().size.y / 2.f});
        sprite.setPosition(PhysicsUnits::toPixels(drawPos));
        sprite.setRotation(sf::degrees(angleDegrees));
        target.draw(sprite);
    } 
    else {
        
    }
}

void GameObject::spawn(const PhysicsWorld &physicsWorld, sf::Vector2f spawnPixels, sf::Vector2f hitboxPixels) {
    if (!physicsWorld.isValid())
        throw std::runtime_error("Invalid World!");
    if(_body && _body->isValid())
        throw std::runtime_error("The player has already been spawned!");

    createBody(physicsWorld, spawnPixels);
    createHitbox(hitboxPixels);
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
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.enableContactEvents = true;
    shapeDef.userData = this;
    onCreateShapeDef(shapeDef);

    b2Polygon box = b2MakeBox(
        PhysicsUnits::toMeters(hitboxPixels.x),
        PhysicsUnits::toMeters(hitboxPixels.y)
    );

    b2ShapeId hitbox = b2CreatePolygonShape(_body->getId(), &shapeDef, &box);
    _body->setHibox(hitbox);
}
