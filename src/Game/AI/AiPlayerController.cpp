#include "Game/AI/AiPlayerController.h"

#include <algorithm>

#include "Game/Behaviours/Moveable.h"
#include "Game/Objects/Player/Player.h"
#include "Game/World/GameWorld.h"

namespace {
float normalizePosition(float value, float minimum, float extent) {
    if (extent <= 0.0f) {
        return 0.0f;
    }
    return std::clamp(2.0f * (value - minimum) / extent - 1.0f, -1.0f, 1.0f);
}

float normalizeVelocity(float value, float scale) {
    return std::clamp(value / scale, -1.0f, 1.0f);
}

float groundedValue(const Player& player) {
    const auto* moveable = player.getBehaviour<Moveable>();
    return moveable && !moveable->isAirbone() ? 1.0f : 0.0f;
}
}

AiPlayerController::AiPlayerController(
    Player& player,
    Player& opponent,
    const GameWorld& world,
    AiPolicy policy
) : _player(player),
    _opponent(opponent),
    _world(world),
    _policy(std::move(policy)) {}

void AiPlayerController::fixedUpdate(float fixedDt) {
    (void)fixedDt;
    if (_player.isPendingDestroy() || _opponent.isPendingDestroy()) {
        return;
    }

    const AiAction action = _policy.decide(observe(_player, _opponent, _world));
    if (action.horizontal < 0) {
        _player.stopMoveRight();
        _player.startMoveLeft();
    } else if (action.horizontal > 0) {
        _player.stopMoveLeft();
        _player.startMoveRight();
    } else {
        _player.stopMoveLeft();
        _player.stopMoveRight();
    }

    if (action.jump) {
        _player.startJump();
    } else {
        _player.stopJump();
    }
}

bool AiPlayerController::isPlayerPendingDestroy() const {
    return _player.isPendingDestroy();
}

AiObservation AiPlayerController::observe(
    const Player& player,
    const Player& opponent,
    const GameWorld& world
) {
    const sf::FloatRect bounds = world.getBounds();
    const sf::Vector2f playerPosition = player.getPosition();
    const sf::Vector2f opponentPosition = opponent.getPosition();
    const sf::Vector2f playerVelocity = player.getVelocity();
    const sf::Vector2f opponentVelocity = opponent.getVelocity();

    AiObservation observation;
    observation.selfX = normalizePosition(
        playerPosition.x, bounds.position.x, bounds.size.x
    );
    observation.selfY = normalizePosition(
        playerPosition.y, bounds.position.y, bounds.size.y
    );
    observation.opponentX = normalizePosition(
        opponentPosition.x, bounds.position.x, bounds.size.x
    );
    observation.opponentY = normalizePosition(
        opponentPosition.y, bounds.position.y, bounds.size.y
    );
    observation.selfVelocityX = normalizeVelocity(playerVelocity.x, 640.0f);
    observation.selfVelocityY = normalizeVelocity(playerVelocity.y, 1600.0f);
    observation.opponentVelocityX = normalizeVelocity(opponentVelocity.x, 640.0f);
    observation.opponentVelocityY = normalizeVelocity(opponentVelocity.y, 1600.0f);
    observation.selfGrounded = groundedValue(player);
    observation.opponentGrounded = groundedValue(opponent);
    return observation;
}
