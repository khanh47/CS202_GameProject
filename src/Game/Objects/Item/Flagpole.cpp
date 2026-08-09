#include "Game/Objects/Item/Flagpole.h"

#include "Game/Behaviours/Animatable.h"
#include "Game/Objects/Player/Player.h"
#include "Game/World/GameWorld.h"
#include "ResourceManager.h"

Flagpole::Flagpole() : Item() {
    configureVisuals(ResourceManager::getInstance().getTexture("goal_flag_spritesheet"));
}

Flagpole::Flagpole(sf::Texture& texture) : Item() {
    configureVisuals(texture);
}

void Flagpole::configureVisuals(sf::Texture& texture) {
    if (auto* animatable = getBehaviour<Animatable>()) {
        animatable->configureVisuals(texture, "flagpole");
    }
}

void Flagpole::onCreateBodyDef(b2BodyDef& def) {
    def.type = b2_staticBody;
}

void Flagpole::onCreateShapeDef(b2ShapeDef& def) {
    def.isSensor = true;
    def.enableContactEvents = false;
    def.enableSensorEvents = true;
    def.density = 0.0f;

    def.filter.categoryBits = 0x0010;
    def.filter.maskBits = 0x0002;
}

void Flagpole::onContact(GameObject& other, const b2ContactData&, b2ShapeId) {
    if (_triggered) {
        return;
    }

    auto* player = dynamic_cast<Player*>(&other);
    if (!player || !player->getGameWorld()) {
        return;
    }

    _triggered = true;
    player->getGameWorld()->reachFlagpole(getPosition());
}
