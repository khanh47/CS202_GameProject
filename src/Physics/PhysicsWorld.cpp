#include "Physics/PhysicsWorld.h"
#include "Game/Objects/GameObject.h"
#include <memory>

PhysicsWorld::PhysicsWorld() {
    b2WorldDef worldDef = b2DefaultWorldDef();

    worldDef.gravity = {0.0f, 9.8f};

    _worldId = b2CreateWorld(&worldDef);
}

PhysicsWorld::~PhysicsWorld() {
    if(isValid()) {
        b2DestroyWorld(_worldId);
        _worldId = b2_nullWorldId;
    }
}

void PhysicsWorld::updateSimulation(const float &fixedDt){
    b2World_Step(_worldId, fixedDt, subSteps);

    b2ContactEvents contactEvents = b2World_GetContactEvents(_worldId);

    // AI SLOP
    /*
    for (int i = 0; i < contactEvents.beginCount; ++i)
    {
        b2ContactBeginTouchEvent* event = &contactEvents.beginEvents[i];
        
        b2ShapeId shapeA = event->shapeIdA;
        b2ShapeId shapeB = event->shapeIdB;

        // b2Manifold manifold = event->manifold; 

        void* objA = b2Shape_GetUserData(shapeA);
        void* objB = b2Shape_GetUserData(shapeB);

        
        if (userData != nullptr) {
            auto* weak_ptr = static_cast<std::weak_ptr<GameObject>*>(userData);
            delete weak_ptr; 
        }
        
    }

    // 2. Handle End Touch Events (Objects stopped touching)
    for (int i = 0; i < contactEvents.endCount; ++i)
    {
        b2ContactEndTouchEvent* event = &contactEvents.endEvents[i];
        // Handle separation logic here
    }

    // 3. Handle Hit Events (High-speed impacts, perfect for sound effects)
    for (int i = 0; i < contactEvents.hitCount; ++i)
    {
        b2ContactHitEvent* event = &contactEvents.hitEvents[i];
        float approachSpeed = event->approachSpeed;
        // Play a sound scaled to the approachSpeed
    }
        */
}