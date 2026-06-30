#include "Physics/PhysicsBody.h"

PhysicsBody::PhysicsBody(const PhysicsWorld &physicsWorld, const b2BodyDef &bodyDef): _worldId(physicsWorld.getId()) {
    _bodyId = b2CreateBody(physicsWorld.getId(), &bodyDef);
}

PhysicsBody::PhysicsBody(PhysicsBody&& other) {
    _worldId = other._worldId;
    _bodyId = other._bodyId;
    other._worldId = b2_nullWorldId;
    other._bodyId = b2_nullBodyId;
}

PhysicsBody& PhysicsBody::operator = (PhysicsBody&& other) {
    if (this != &other) {
        destroy();

        _worldId = other._worldId;
        _bodyId = other._bodyId;

        other._worldId = b2_nullWorldId;
        other._bodyId = b2_nullBodyId;
    }

    return *this;
}

PhysicsBody::~PhysicsBody() {
    destroy();
}

void PhysicsBody::destroy() {
    if(isValid()){
        b2DestroyBody(_bodyId);
        _bodyId = b2_nullBodyId;
    }
}

void PhysicsBody::setHibox(b2ShapeId shapeId) {
    _shapeId = shapeId;
}

b2ShapeId PhysicsBody::getHitbox() const {
    return _shapeId;
}