#include "Game/AI/AiPlayerController.h"

#include "Game/Behaviours/Moveable.h"
#include "Game/Objects/Player/Player.h"
#include "Game/World/GameWorld.h"

AiObservation AiPlayerController::observe(
    const Player& self,
    const Player& opponent,
    const GameWorld& world
) {
    const sf::FloatRect bounds = world.getBounds();
    const float arenaCenterX = bounds.position.x + bounds.size.x * 0.5f;
    const sf::Vector2f selfPosition = self.getPosition();
    const sf::Vector2f opponentPosition = opponent.getPosition();
    const Moveable* moveable = self.getBehaviour<Moveable>();

    return {
        selfPosition.x - arenaCenterX,
        selfPosition.y,
        opponentPosition.x - arenaCenterX,
        opponentPosition.y,
        opponent.getVelocity().y,
        moveable && !moveable->isAirbone() ? 1.0f : 0.0f,
        bounds.size.x * 0.5f,
        self.getHitboxPixels().x * 0.5f
    };
}
