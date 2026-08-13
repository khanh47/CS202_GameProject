#pragma once

#include "Game/AI/AiPolicy.h"
#include "Game/UserInput/IPlayerController.h"

class GameWorld;
class Player;

class AiPlayerController : public IPlayerController {
public:
    AiPlayerController(
        Player& player,
        Player& opponent,
        const GameWorld& world,
        AiPolicy policy
    );

    void fixedUpdate(float fixedDt) override;
    bool isPlayerPendingDestroy() const override;

    static AiObservation observe(
        const Player& player,
        const Player& opponent,
        const GameWorld& world
    );

private:
    Player& _player;
    Player& _opponent;
    const GameWorld& _world;
    AiPolicy _policy;
};
