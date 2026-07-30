#include "Game/World/WorldInteractionSystem.h"

#include <memory>

#include "Game/Behaviours/Moveable.h"
#include "Game/Objects/Enemy/Enemy.h"
#include "Game/Objects/GameObject.h"
#include "Game/Objects/Item/FireballPool.h"
#include "Game/Objects/Item/FireFlower.h"
#include "Game/Objects/Player/Player.h"
#include "Game/World/GameWorld.h"
#include "Game/World/WorldObjectStore.h"

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

        objectA->onContact(*objectB);
        objectB->onContact(*objectA);
    }
}

void WorldInteractionSystem::processSensors(b2SensorEvents events) {
    for (int index = 0; index < events.endCount; ++index) {
        const b2ShapeId sensor = events.endEvents[index].sensorShapeId;
        if (!b2Shape_IsValid(sensor)) {
            continue;
        }
        auto* owner = static_cast<GameObject*>(b2Shape_GetUserData(sensor));
        if (auto* moveable = dynamic_cast<Moveable*>(owner)) {
            moveable->endGroundContact();
        }
    }

    for (int index = 0; index < events.beginCount; ++index) {
        const b2ShapeId sensor = events.beginEvents[index].sensorShapeId;
        if (!b2Shape_IsValid(sensor)) {
            continue;
        }
        auto* owner = static_cast<GameObject*>(b2Shape_GetUserData(sensor));
        if (auto* moveable = dynamic_cast<Moveable*>(owner)) {
            moveable->beginGroundContact();
        }
    }
}

void WorldInteractionSystem::processObjectInteractions(
    WorldObjectStore& objectStore,
    FireballPool& fireballPool,
    GameWorld& gameWorld
) {
    (void)gameWorld;
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
                fireball->onContact(*enemy);
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
            player->onContact(*fireFlower);
        }
    }
}
