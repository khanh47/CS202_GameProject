#pragma once

class GameWorld;
class Player;

struct AiAction {
    int horizontal = 0;
    bool jump = false;
};

struct AiObservation {
    float selfX = 0.0f;
    float selfY = 0.0f;
    float opponentX = 0.0f;
    float opponentY = 0.0f;
    float opponentVelocityY = 0.0f;
    float selfGrounded = 0.0f;
    float arenaHalfWidth = 0.0f;
    float selfHalfWidth = 0.0f;
};

class AiPlayerController {
public:
    static AiObservation observe(
        const Player& self,
        const Player& opponent,
        const GameWorld& world
    );
};
