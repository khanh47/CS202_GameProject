#include "Game/Objects/Enemy/ConcreteEnemy/Goomba.h"

Goomba::Goomba() : Enemy() {}

Goomba::Goomba(sf::Texture& texture, const std::string& animationSetId) : Enemy(texture, animationSetId) {
}