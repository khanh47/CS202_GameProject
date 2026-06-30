#include "Physics/PhysicsWorld.h"

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
}