#pragma once

class Moveable {
public:
    void startMoveLeft() { _movingLeft = true; _facingLeft = true; _facingRight = false; }
    void stopMoveLeft() { _movingLeft = false; }
    void startMoveRight() { _movingRight = true; _facingLeft = false; _facingRight = true; }
    void stopMoveRight() { _movingRight = false; } 
    void startJump() { _jumping = true; }
    void stopJump() { _jumping = false; }

    bool isMovingRight() const { return _movingRight; }
    bool isMovingLeft() const { return _movingLeft; }
    bool isFacingLeft() const { return _facingLeft; }
    bool isFacingRight() const { return _facingRight; }
    bool isJumping() const { return _jumping; }

private:
    bool _facingLeft = false;
    bool _facingRight = false;
    bool _movingLeft = false;
    bool _movingRight = false;
    bool _jumping = false;
};
