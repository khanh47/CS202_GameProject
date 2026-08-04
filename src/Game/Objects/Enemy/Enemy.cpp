#include "Game/Objects/Enemy/Enemy.h"
#include "Game/Behaviours/Animatable.h"
#include "Game/Behaviours/Damageable.h"
#include "Game/Objects/Block/Block.h"
#include "Game/Objects/GameObject.h"
#include "Game/World/TerrainSeamFilter.h"
#include "Physics/PhysicsUnits.h"
#include "box2d/box2d.h"
#include <cmath>
#include <iostream>
#include <memory>


namespace {
struct SensorQueryContext {
    const b2ShapeId* sensorShapeId;
    int overlapCount = 0;
};

bool sensorOverlapCallback(b2ShapeId shapeId, void* context) {
    auto* query = static_cast<SensorQueryContext*>(context);
    if (query->sensorShapeId != nullptr
        && b2Shape_IsValid(*query->sensorShapeId)
        && B2_ID_EQUALS(shapeId, *query->sensorShapeId)) {
        return true;
    }
    ++query->overlapCount;
    return true;
}
}

Enemy::Enemy() : GameObject() {
    animatable = std::make_unique<Animatable>();
    damageable = std::make_unique<Damageable>(50);
}

Enemy::Enemy(sf::Texture& texture) : Enemy() {
    animatable->configureVisuals(texture);
}

Enemy::Enemy(sf::Texture &texture, const std::string& animationSetId) : Enemy() {
    animatable->configureVisuals(texture, animationSetId);
}

Enemy::~Enemy() {
}

void Enemy::setSupportGrid(const TerrainSeamFilter* filter, float cellSize) {
    _supportGrid = filter;
    _supportCellSize = cellSize;
}

bool Enemy::isSupportedByGrid(int& outCol, int& outRow) const {
    if (!_supportGrid || !hasValidBody()) {
        outCol = 0;
        outRow = 0;
        return false;
    }

    const b2Vec2 posMeters = b2Body_GetPosition(_body->getId());
    const sf::Vector2f posPx = PhysicsUnits::toPixels(posMeters);

    constexpr float sensorForwardPixels = 2.0f;
    constexpr float sensorBelowFeetPixels = 2.0f;

    const float probeX = posPx.x + _sensorDirection * (_hitboxPixels.x * 0.5f + sensorForwardPixels);
    const float probeY = posPx.y + _hitboxPixels.y * 0.5f + sensorBelowFeetPixels;

    outRow = static_cast<int>(std::floor((probeY + _supportCellSize * 0.5f) / _supportCellSize));
    outCol = static_cast<int>(std::floor(probeX / _supportCellSize));

    return _supportGrid->isCellOccupied(outCol - 1, outRow)
        || _supportGrid->isCellOccupied(outCol, outRow)
        || _supportGrid->isCellOccupied(outCol + 1, outRow);
}

void Enemy::onCreateBodyDef(b2BodyDef& def) {
    def.type = b2_dynamicBody;
    def.motionLocks.angularZ = true;
}

void Enemy::onCreateShapeDef(b2ShapeDef& def) {
    def.density = 1.0f;
    def.material.friction = 0.0f;
    
    // Category 0x0008 (Enemy)
    def.filter.categoryBits = 0x0008;
}

void Enemy::onUpdateVisuals(float deltaTime) {
    bool facingLeft = hasValidBody() && b2Body_GetLinearVelocity(_body->getId()).x < 0.f;
    animatable->updateVisualState(deltaTime, _hitboxPixels, facingLeft);
}

void Enemy::onRenderVisual(sf::RenderTarget& target, const sf::Vector2f& position, float angleDegrees) {
    animatable->renderVisualState(target, position, angleDegrees);
}

void Enemy::updateSimulation(const float &fixedDt) {
    (void)fixedDt;

    ++_simTick;

    if (hasSensor()) {
        bool isTouching = false;
        int col = 0, row = 0;

        if (_supportGrid) {
            isTouching = isSupportedByGrid(col, row);
        } else {
            const SensorProbeResult probe = probeSensor();
            isTouching = probe.touching;
        }

        if (isTouching != _wasTouching) {
            _wasTouching = isTouching;
            const b2Vec2 pos = b2Body_GetPosition(_body->getId());
            if (_supportGrid) {
                std::cout << "[Sensor] tick=" << _simTick
                          << " body=(" << pos.x << "," << pos.y << ")"
                          << " dir=" << _sensorDirection
                          << " grid=(" << col << "," << row << ")"
                          << " supported=" << (isTouching ? 1 : 0)
                          << " unsupported=" << _unsupportedSteps
                          << " speed=" << _moveSpeed << std::endl;
            } else {
                const SensorProbeResult probe = probeSensor();
                const sf::Vector2f lower = PhysicsUnits::toPixels(probe.worldAABB.lowerBound);
                const sf::Vector2f upper = PhysicsUnits::toPixels(probe.worldAABB.upperBound);
                std::cout << "[Sensor] tick=" << _simTick
                          << " body=(" << pos.x << "," << pos.y << ")"
                          << " dir=" << _sensorDirection
                          << " probe=(" << lower.x << "," << lower.y << ")-("
                                       << upper.x << "," << upper.y << ")"
                          << " overlaps=" << probe.overlapCount
                          << " touching=" << (probe.touching ? 1 : 0)
                          << " unsupported=" << _unsupportedSteps
                          << " speed=" << _moveSpeed << std::endl;
            }
        }

        if (isTouching) {
            _unsupportedSteps = 0;
        } else {
            ++_unsupportedSteps;
            if (_unsupportedSteps >= unsupportedStepsBeforeTurn) {
                _moveSpeed = -_moveSpeed;
                _unsupportedSteps = 0;
                flipSensorDirection();
                std::cout << "[Sensor] TURN(ledge) tick=" << _simTick
                          << " speed=" << _moveSpeed
                          << " dir=" << _sensorDirection << std::endl;
            }
        }
    }

    b2Vec2 velocity = b2Body_GetLinearVelocity(_body->getId());
    velocity.x = _moveSpeed;
    b2Body_SetLinearVelocity(_body->getId(), velocity);
}

Enemy::SensorProbeResult Enemy::probeSensor() const {
    SensorProbeResult result;
    if (!b2Shape_IsValid(_sensorShapeId)) {
        return result;
    }

    result.worldAABB = b2Shape_GetAABB(_sensorShapeId);

    b2QueryFilter filter = b2DefaultQueryFilter();
    filter.categoryBits = 0x0001;
    filter.maskBits = 0x0001;

    SensorQueryContext query{&_sensorShapeId, 0};
    b2World_OverlapAABB(
        b2Shape_GetWorld(_sensorShapeId),
        b2Pos_zero,
        result.worldAABB,
        filter,
        sensorOverlapCallback,
        &query
    );

    result.overlapCount = query.overlapCount;
    result.touching = query.overlapCount > 0;
    return result;
}

void Enemy::onContact(GameObject& other, const b2ContactData& contactData, b2ShapeId ownShape) {
    // if (auto* block = dynamic_cast<Block*>(&other)) {
    //     if (b2Shape_IsValid(ownShape)) {
    //         b2Vec2 normal = contactData.manifold.normal;
    //         if (!B2_ID_EQUALS(contactData.shapeIdA, ownShape)) {
    //             normal = {-normal.x, -normal.y};
    //         }
    //         const bool movingInto = (normal.x > 0.0f && _moveSpeed > 0.0f)
    //                              || (normal.x < 0.0f && _moveSpeed < 0.0f);
    //         const bool horizontal = contactData.manifold.pointCount > 0
    //                              && std::abs(normal.x) > std::abs(normal.y);
    //         std::cout << "[Sensor] CONTACT tick=" << _simTick
    //                   << " normal=(" << normal.x << "," << normal.y << ")"
    //                   << " points=" << contactData.manifold.pointCount
    //                   << " movingInto=" << (movingInto ? 1 : 0)
    //                   << " flipped=" << (horizontal && movingInto ? 1 : 0)
    //                   << std::endl;
    //         if (horizontal && movingInto) {
    //             _moveSpeed = -_moveSpeed;
    //             flipSensorDirection();
    //         }
    //     }
    // }
}
