#pragma once

#include "Game/AI/AiPlayerController.h"

class GameWorld;
class Player;

class HeuristicAiController {
public:
    HeuristicAiController(
        Player& self,
        Player& opponent,
        const GameWorld& world
    );

    void fixedUpdate(float fixedDt);
    bool isPlayerEliminated() const;

    AiAction decide(const AiObservation& observation);

private:
    enum class EdgeRecovery {
        None,
        MoveLeft,
        MoveRight
    };

    static constexpr float alignedThreshold = 48.0f;
    static constexpr float stompThreatHorizontalDistance = 144.0f;
    static constexpr float stompThreatVerticalDistance = 16.0f;
    static constexpr float stompThreatLaunchHeightTolerance = 48.0f;
    static constexpr float stompThreatRiseSpeed = -80.0f;
    static constexpr float edgeSafetyInset = 32.0f;
    static constexpr float edgeRecoveryDistance = 128.0f;

    Player& _self;
    Player& _opponent;
    const GameWorld& _world;
    EdgeRecovery _edgeRecovery = EdgeRecovery::None;
    int _lastHorizontal = -1;
};
