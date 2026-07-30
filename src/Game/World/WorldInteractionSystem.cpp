#include "Game/World/WorldInteractionSystem.h"

#include <algorithm>
#include <memory>

#include "Game/Behaviours/Moveable.h"
#include "Game/Objects/Enemy/Enemy.h"
#include "Game/Objects/GameObject.h"
#include "Game/Objects/Item/FireballPool.h"
#include "Game/Objects/Item/Item.h"
#include "Game/Objects/Item/FireFlower.h"
#include "Game/Objects/Player/Player.h"
#include "Game/World/GameWorld.h"
#include "Game/World/WorldObjectStore.h"
#include "box2d/box2d.h"

namespace {
constexpr float minimumGroundNormalY = 0.9f;
constexpr float minimumSupportOverlapMeters = 1.0f / 64.0f;
constexpr uint64_t environmentCategory = 0x0001;

Player* getPlayer(GameObject* objectA, GameObject* objectB) {
    if (auto* player = dynamic_cast<Player*>(objectA)) {
        return player;
    }
    return dynamic_cast<Player*>(objectB);
}

void processGroundContactBegin(
    const b2ContactBeginTouchEvent& event,
    GameObject* objectA,
    GameObject* objectB
) {
    Player* player = getPlayer(objectA, objectB);
    if (!player || !b2Contact_IsValid(event.contactId)) {
        return;
    }

    const b2ShapeId playerShape =
        player == objectA ? event.shapeIdA : event.shapeIdB;
    const b2ShapeId otherShape =
        player == objectA ? event.shapeIdB : event.shapeIdA;
    if ((b2Shape_GetFilter(otherShape).categoryBits
         & environmentCategory) == 0) {
        return;
    }

    const b2ContactData contact = b2Contact_GetData(event.contactId);
    b2Vec2 playerToOther = contact.manifold.normal;
    if (!B2_ID_EQUALS(contact.shapeIdA, playerShape)) {
        playerToOther = {-playerToOther.x, -playerToOther.y};
    }

    const b2AABB playerBounds = b2Shape_GetAABB(playerShape);
    const b2AABB supportBounds = b2Shape_GetAABB(otherShape);
    const float horizontalOverlap =
        std::min(playerBounds.upperBound.x, supportBounds.upperBound.x)
        - std::max(playerBounds.lowerBound.x, supportBounds.lowerBound.x);

    if (contact.manifold.pointCount > 0
        && playerToOther.y >= minimumGroundNormalY
        && horizontalOverlap >= minimumSupportOverlapMeters) {
        player->beginGroundContact(otherShape);
    }
}

void processGroundContactEnd(
    const b2ContactEndTouchEvent& event,
    GameObject* objectA,
    GameObject* objectB
) {
    Player* player = getPlayer(objectA, objectB);
    if (!player) {
        return;
    }

    const b2ShapeId otherShape =
        player == objectA ? event.shapeIdB : event.shapeIdA;
    player->endGroundContact(otherShape);
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

        processGroundContactBegin(event, objectA, objectB);

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

    for (int index = 0; index < events.endCount; ++index) {
        const b2ContactEndTouchEvent& event = events.endEvents[index];
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
        if (objectA && objectB) {
            processGroundContactEnd(event, objectA, objectB);
        }
    }
}

void WorldInteractionSystem::processSensors(b2SensorEvents events) {
    (void)events;
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
