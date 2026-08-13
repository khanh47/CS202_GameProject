#include "Game/AI/HeuristicAiController.h"

#include <algorithm>
#include <cmath>

#include "Game/AI/AiPlayerController.h"
#include "Game/Behaviours/Moveable.h"
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
    if (_self.isPendingDestroy() || _opponent.isPendingDestroy()) {
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

bool HeuristicAiController::isPlayerPendingDestroy() const {
    return _self.isPendingDestroy();
}

AiAction HeuristicAiController::decide(const AiObservation& observation) {
    AiAction action;

    const float relativeX = observation.opponentX - observation.selfX;
    const float relativeY = observation.opponentY - observation.selfY;
    // SFML's Y axis points down, so a negative relativeY means the
    // opponent is above the heuristic player.
    const bool opponentAbove = relativeY < 0.0f;
    const bool aligned = std::abs(relativeX) < alignedThreshold;

    int horizontal = directionToward(relativeX);
    if (opponentAbove && std::abs(relativeX) < evadeDistance) {
        // The opponent is above and threatening a stomp; back away instead
        // of charging underneath them.
        horizontal = -horizontal;
    }

    const bool nearRightEdge = observation.selfX > edgeMargin;
    const bool nearLeftEdge = observation.selfX < -edgeMargin;
    if (nearRightEdge && horizontal > 0) {
        horizontal = -1;
    } else if (nearLeftEdge && horizontal < 0) {
        horizontal = 1;
    }

    action.horizontal = horizontal;
    action.jump = observation.selfGrounded > 0.0f && aligned;
    return action;
}
