#pragma once

#include "Game/UserInput/IPlayerController.h"
#include "Game/AI/AiPolicy.h"

class GameWorld;
class Player;

class HeuristicAiController : public IPlayerController {
public:
    HeuristicAiController(
        Player& self,
        Player& opponent,
        const GameWorld& world
    );

    void fixedUpdate(float fixedDt) override;
    bool isPlayerPendingDestroy() const override;

    static AiAction decide(const AiObservation& observation);

    static constexpr float alignedThreshold = 0.15f;
    static constexpr float evadeDistance = 0.6f;
    static constexpr float edgeMargin = 1.0f;

private:
    Player& _self;
    Player& _opponent;
    const GameWorld& _world;
};
