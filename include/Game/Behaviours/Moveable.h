#pragma once

#include <algorithm>
#include <vector>

#include <box2d/id.h>

class Moveable {
public:
    void startMoveLeft() { _movingLeft = true; _facingLeft = true; }
    void stopMoveLeft() { _movingLeft = false; }
    void startMoveRight() { _movingRight = true; _facingLeft = false; }
    void stopMoveRight() { _movingRight = false; } 
    void startJump() { if(!isAirbone()) _jumping = true; }
    void stopJump() { _jumping = false; }

    void beginGroundContact() {
        ++_groundContactCount;
        _groundContactGraceFramesRemaining = kGroundContactGraceFrames;
    }

    void beginGroundContact(b2ShapeId visitor) {
        const auto existing = std::find_if(
            _groundVisitors.begin(),
            _groundVisitors.end(),
            [visitor](b2ShapeId current) {
                return B2_ID_EQUALS(current, visitor);
            }
        );
        if (existing == _groundVisitors.end()) {
            _groundVisitors.push_back(visitor);
            beginGroundContact();
        }
    }

    void endGroundContact() {
        if (_groundContactCount > 0) {
            --_groundContactCount;
        }
    }

    void endGroundContact(b2ShapeId visitor) {
        const auto existing = std::find_if(
            _groundVisitors.begin(),
            _groundVisitors.end(),
            [visitor](b2ShapeId current) {
                return B2_ID_EQUALS(current, visitor);
            }
        );
        if (existing != _groundVisitors.end()) {
            _groundVisitors.erase(existing);
            endGroundContact();
        }
    }

    void resetGroundContacts() {
        _groundContactCount = 0;
        _groundContactGraceFramesRemaining = 0;
        _groundVisitors.clear();
    }

    void finalizeGroundContacts() {
        if (_groundContactCount > 0) {
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
        return _groundContactCount <= 0 && _groundContactGraceFramesRemaining <= 0;
    }
    

private:
    static constexpr int kGroundContactGraceFrames = 1;

    bool _facingLeft = false;
    bool _movingLeft = false;
    bool _movingRight = false;
    bool _jumping = false;
    int _groundContactCount = 0;
    int _groundContactGraceFramesRemaining = 0;
    std::vector<b2ShapeId> _groundVisitors;
};
