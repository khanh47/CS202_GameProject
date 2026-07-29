#pragma once

#include "box2d/id.h"
#include <iostream>
#include <set>

struct b2ShapeIdLessThan {
    bool operator()(const b2ShapeId& a, const b2ShapeId& b) const {
        if (a.index1 != b.index1) return a.index1 < b.index1;
        if (a.world0 != b.world0) return a.world0 < b.world0;
        return a.generation < b.generation;
    }
};

class Moveable {
public:
    void startMoveLeft() { _movingLeft = true; _facingLeft = true; }
    void stopMoveLeft() { _movingLeft = false; }
    void startMoveRight() { _movingRight = true; _facingLeft = false; }
    void stopMoveRight() { _movingRight = false; } 
    void startJump() { if(!isAirbone()) _jumping = true; }
    void stopJump() { _jumping = false; }

    void addSensorVisitor(b2ShapeId visitor) {
        _pendingSensorVisitorRemovals.erase(visitor);
        _sensorVisitors.insert(visitor);
        _groundContactGraceFramesRemaining = kGroundContactGraceFrames;
    }
    void queueSensorVisitorRemoval(b2ShapeId visitor) { _pendingSensorVisitorRemovals.insert(visitor); }
    void finalizeSensorVisitors() {
        for (const b2ShapeId& visitor : _pendingSensorVisitorRemovals) {
            _sensorVisitors.erase(visitor);
        }
        _pendingSensorVisitorRemovals.clear();

        if (!_sensorVisitors.empty()) {
            _groundContactGraceFramesRemaining = kGroundContactGraceFrames;
        } else if (_groundContactGraceFramesRemaining > 0) {
            --_groundContactGraceFramesRemaining;
        }
    }
    
    bool isMovingRight() const { return _movingRight; }
    bool isMovingLeft() const { return _movingLeft; }
    bool isJumping() const { return _jumping; }

    bool isFacingLeft() const { return _facingLeft; }
    bool isAirbone() const {
        return _sensorVisitors.empty() && _groundContactGraceFramesRemaining <= 0;
    }
    

private:
    static constexpr int kGroundContactGraceFrames = 4;

    bool _facingLeft = false;
    bool _movingLeft = false;
    bool _movingRight = false;
    bool _jumping = false;
    int _groundContactGraceFramesRemaining = 0;

    std::set<b2ShapeId, b2ShapeIdLessThan> _sensorVisitors;
    std::set<b2ShapeId, b2ShapeIdLessThan> _pendingSensorVisitorRemovals;
};
