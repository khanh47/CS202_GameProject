#include "Game/Objects/Enemy/ConcreteEnemy/Goomba.h"

Goomba::Goomba() : Enemy() {}

Goomba::Goomba(sf::Texture& texture) : Enemy(texture) {
    // Configures Animator using "goomba" spritesheet set from AnimationLibrary
    configureVisuals(texture, "goomba");
}

void Goomba::onCreateBodyDef(b2BodyDef& def) {
    def.type = b2_dynamicBody; // Goombas move, so they must be dynamic
    def.motionLocks.angularZ = true; // Lock rotation so they don't roll over
}

void Goomba::onCreateShapeDef(b2ShapeDef& def) {
    def.density = 1.0f;
    def.material.friction = 0.2f; // Slight friction so they slide naturally
}