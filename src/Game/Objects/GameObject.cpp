#include "Game/Objects/GameObject.h"
#include "Physics/PhysicsUnits.h"

void GameObject::updateSimulation(const float &fixedDt) {

}

void GameObject::updateVisuals(float deltaTime) {

}

void GameObject::render(sf::RenderTarget &target) { // DEFINITELY NEEDS TO BE REFRACTORED
    if(!_body->isValid()) return;

    b2Vec2 position = b2Body_GetPosition(_body->getId());
    b2Rot rotation = b2Body_GetRotation(_body->getId());
    float angleDegrees = b2Rot_GetAngle(rotation) * (180.f / 3.14159265f);

    b2ShapeId shape = _body->getHitbox();
    b2ShapeType type = b2Shape_GetType(shape);

    if (_sprite.has_value()) {
        sf::Sprite& sprite = _sprite.value();
        
        if (type == b2_polygonShape) {
            b2Polygon polygon = b2Shape_GetPolygon(shape);
            float widthMeters = std::abs(polygon.vertices[0].x - polygon.vertices[2].x);
            float heightMeters = std::abs(polygon.vertices[0].y - polygon.vertices[2].y);
            sf::Vector2f sizePixels = PhysicsUnits::toPixels({widthMeters, heightMeters});
            
            sf::Vector2u texSize = sprite.getTexture().getSize(); 
            sprite.setScale({sizePixels.x / texSize.x, sizePixels.y / texSize.y});
        } 
        else if (type == b2_circleShape) {
            b2Circle circle = b2Shape_GetCircle(shape);
            float radiusPixels = PhysicsUnits::toPixels(circle.radius);
            float diameterPixels = radiusPixels * 2.f;
            
            sf::Vector2u texSize = sprite.getTexture().getSize();
            sprite.setScale({diameterPixels / texSize.x, diameterPixels / texSize.y});
        }

        sprite.setOrigin({sprite.getLocalBounds().size.x / 2.f, sprite.getLocalBounds().size.y / 2.f});
        sprite.setPosition(PhysicsUnits::toPixels(position));
        sprite.setRotation(sf::degrees(angleDegrees));
        target.draw(sprite);
    } 
    else {
        // rect
        if (type == b2_polygonShape) {
            b2Polygon polygon = b2Shape_GetPolygon(shape);
            
            float widthMeters = std::abs(polygon.vertices[0].x - polygon.vertices[2].x);
            float heightMeters = std::abs(polygon.vertices[0].y - polygon.vertices[2].y);
            sf::Vector2f sizePixels = PhysicsUnits::toPixels({widthMeters, heightMeters});

            sf::RectangleShape fallbackRect(sizePixels);
            fallbackRect.setOrigin({sizePixels.x / 2.f, sizePixels.y / 2.f});
            fallbackRect.setPosition(PhysicsUnits::toPixels(position));
            fallbackRect.setRotation(sf::degrees(angleDegrees));
            fallbackRect.setFillColor(sf::Color::Magenta);

            target.draw(fallbackRect);
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
    }
}