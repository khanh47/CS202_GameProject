#pragma once

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

    void endGroundContact() {
        if (_groundContactCount > 0) {
            --_groundContactCount;
        }
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
};
