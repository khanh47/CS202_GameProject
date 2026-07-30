#include "Game/World/WorldInteractionSystem.h"

#include <memory>

#include "Game/Behaviours/Moveable.h"
#include "Game/Objects/Block/Block.h"
#include "Game/Objects/Enemy/Enemy.h"
#include "Game/Objects/GameObject.h"
#include "Game/Objects/Item/FireFlower.h"
#include "Game/Objects/Item/Fireball.h"
#include "Game/Objects/Item/FireballPool.h"
#include "Game/Objects/Item/Item.h"
#include "Game/Objects/Player/Player.h"
#include "Game/World/GameWorld.h"
#include "Game/World/WorldObjectStore.h"
#include "Physics/PhysicsUnits.h"
#include "box2d/box2d.h"

namespace {
constexpr float groundSurfaceTolerancePixels = 16.0f;

bool isSurfaceBelowFeet(b2ShapeId sensor, b2ShapeId visitor) {
    if (!b2Shape_IsValid(visitor)) {
        return false;
    }

    const b2AABB feetBounds = b2Shape_GetAABB(sensor);
    const b2AABB surfaceBounds = b2Shape_GetAABB(visitor);
    const float tolerance =
        PhysicsUnits::toMeters(groundSurfaceTolerancePixels);
    const float surfaceTop = surfaceBounds.lowerBound.y;

    return surfaceTop >= feetBounds.lowerBound.y - tolerance
        && surfaceTop <= feetBounds.upperBound.y + tolerance;
}
}

void WorldInteractionSystem::processContacts(b2ContactEvents events) {
    for (int index = 0; index < events.beginCount; ++index) {
        const b2ContactBeginTouchEvent& event = events.beginEvents[index];
        if (!b2Shape_IsValid(event.shapeIdA)
            || !b2Shape_IsValid(event.shapeIdB)) {
            continue;
        }

        auto* objectA = static_cast<GameObject*>(
            b2Shape_GetUserData(event.shapeIdA)
        );
        auto* objectB = static_cast<GameObject*>(
            b2Shape_GetUserData(event.shapeIdB)
        );
        if (!objectA || !objectB) {
            continue;
        }

        auto* fireball = dynamic_cast<Fireball*>(objectA);
        GameObject* other = objectB;
        if (!fireball) {
            fireball = dynamic_cast<Fireball*>(objectB);
            other = objectA;
        }

        if (!fireball || !fireball->isActive()
            || !dynamic_cast<Block*>(other)) {
            continue;
        }

        const sf::Vector2f difference =
            fireball->getPosition() - other->getPosition();
        if (difference.y < -16.0f) {
            fireball->triggerBounce();
        } else {
            fireball->deactivate();
        }
    }
}

void WorldInteractionSystem::processSensors(b2SensorEvents events) {
    for (int index = 0; index < events.endCount; ++index) {
        const b2ShapeId sensor = events.endEvents[index].sensorShapeId;
        if (!b2Shape_IsValid(sensor)) {
            continue;
        }
        
        const b2ShapeId visitorShape = events.beginEvents[index].visitorShapeId;
        GameObject* owner = static_cast<GameObject*>(b2Shape_GetUserData(sensor));
        GameObject* visitor = static_cast<GameObject*>(b2Shape_GetUserData(visitorShape));

        if (auto* moveable = dynamic_cast<Moveable*>(owner)) {
            moveable->endGroundContact(events.endEvents[index].visitorShapeId);
        }
    }

    for (int index = 0; index < events.beginCount; ++index) {
        const b2ShapeId sensor = events.beginEvents[index].sensorShapeId;
        if (!b2Shape_IsValid(sensor)) {
            continue;
        }

        const b2ShapeId visitorShape = events.beginEvents[index].visitorShapeId;
        GameObject* owner = static_cast<GameObject*>(b2Shape_GetUserData(sensor));
        GameObject* visitor = static_cast<GameObject*>(b2Shape_GetUserData(visitorShape));

        if (auto* moveable = dynamic_cast<Moveable*>(owner);
            moveable && isSurfaceBelowFeet(sensor, visitorShape)) {
            moveable->beginGroundContact(visitorShape);
        }
    }
}

void WorldInteractionSystem::processObjectInteractions(
    WorldObjectStore& objectStore,
    FireballPool& fireballPool,
    GameWorld& gameWorld
) {
    const auto& objects = objectStore.objects();
    for (const std::shared_ptr<Fireball>& fireball : fireballPool.getPool()) {
        if (!fireball || !fireball->isActive()) {
            continue;
        }
        for (const std::shared_ptr<GameObject>& object : objects) {
            if (!object || object->isPendingDestroy()) {
                continue;
            }
            const auto enemy = std::dynamic_pointer_cast<Enemy>(object);
            if (!enemy) {
                continue;
            }

            const sf::Vector2f difference =
                fireball->getPosition() - enemy->getPosition();
            constexpr float combinedRadius = 19.0f + 32.0f;
            if (difference.x * difference.x + difference.y * difference.y
                < combinedRadius * combinedRadius) {
                enemy->destroy();
                fireball->deactivate();
                break;
            }
        }
    }

    const auto player = std::dynamic_pointer_cast<Player>(
        objectStore.getPrimaryPlayer()
    );
    if (!player) {
        return;
    }

    for (const std::shared_ptr<GameObject>& object : objects) {
        if (!object || object->isPendingDestroy()) {
            continue;
        }
        const auto fireFlower = std::dynamic_pointer_cast<FireFlower>(object);
        if (!fireFlower) {
            continue;
        }
        const sf::Vector2f difference =
            player->getPosition() - fireFlower->getPosition();
        if (difference.x * difference.x + difference.y * difference.y
            < 45.0f * 45.0f) {
            player->startFireTransformation(gameWorld, 1.0f);
            fireFlower->destroy();
        }
    }
}
