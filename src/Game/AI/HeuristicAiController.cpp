#include "Game/AI/HeuristicAiController.h"

#include <algorithm>
#include <cmath>

#include "Game/Objects/Player/Player.h"
#include "Game/World/GameWorld.h"

namespace {
int directionToward(float relativeX) {
    if (relativeX > 0.0f) {
        return 1;
    }
    if (relativeX < 0.0f) {
        return -1;
    }
    return 0;
}
}

HeuristicAiController::HeuristicAiController(
    Player& self,
    Player& opponent,
    const GameWorld& world
) : _self(self),
    _opponent(opponent),
    _world(world) {}

void HeuristicAiController::fixedUpdate(float fixedDt) {
    (void)fixedDt;
    if (_self.isEliminated() || _opponent.isEliminated()) {
        _self.stopMoveLeft();
        _self.stopMoveRight();
        _self.stopJump();
        return;
    }

    const AiAction action = decide(
        AiPlayerController::observe(_self, _opponent, _world)
    );
    if (action.horizontal < 0) {
        _self.stopMoveRight();
        _self.startMoveLeft();
    } else if (action.horizontal > 0) {
        _self.stopMoveLeft();
        _self.startMoveRight();
    } else {
        _self.stopMoveLeft();
        _self.stopMoveRight();
    }

    if (action.jump) {
        _self.startJump();
    } else {
        _self.stopJump();
    }
}

bool HeuristicAiController::isPlayerEliminated() const {
    return _self.isEliminated();
}

AiAction HeuristicAiController::decide(const AiObservation& observation) {
    AiAction action;

    const float relativeX = observation.opponentX - observation.selfX;
    const float relativeY = observation.opponentY - observation.selfY;
    const bool aligned = std::abs(relativeX) < alignedThreshold;
    const bool stompThreat =
        relativeY < -stompThreatVerticalDistance
        && std::abs(relativeX) < stompThreatHorizontalDistance
        && observation.opponentVelocityY > stompThreatFallSpeed;

    const float dangerMargin = std::max(
        0.0f,
        observation.arenaHalfWidth
            - observation.selfHalfWidth
            - edgeSafetyInset
    );
    const float recoveredMargin = std::max(
        0.0f,
        dangerMargin - edgeRecoveryDistance
    );

    if (_edgeRecovery == EdgeRecovery::None) {
        if (observation.selfX >= dangerMargin) {
            _edgeRecovery = EdgeRecovery::MoveLeft;
        } else if (observation.selfX <= -dangerMargin) {
            _edgeRecovery = EdgeRecovery::MoveRight;
        }
    } else if (_edgeRecovery == EdgeRecovery::MoveLeft
               && observation.selfX <= recoveredMargin) {
        _edgeRecovery = EdgeRecovery::None;
    } else if (_edgeRecovery == EdgeRecovery::MoveRight
               && observation.selfX >= -recoveredMargin) {
        _edgeRecovery = EdgeRecovery::None;
    }

    if (_edgeRecovery != EdgeRecovery::None) {
        action.horizontal = _edgeRecovery == EdgeRecovery::MoveLeft ? -1 : 1;
        action.jump = false;
        _lastHorizontal = action.horizontal;
        return action;
    }

    int horizontal = directionToward(relativeX);
    if (stompThreat) {
        if (horizontal == 0) {
            horizontal = observation.selfX > 0.0f
                ? -1
                : observation.selfX < 0.0f
                    ? 1
                    : -_lastHorizontal;
        } else {
            horizontal = -horizontal;
        }
    }

    action.horizontal = horizontal;
    action.jump = !stompThreat
        && observation.selfGrounded > 0.0f
        && aligned;
    if (horizontal != 0) {
        _lastHorizontal = horizontal;
    }
    return action;
}
