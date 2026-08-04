#include "Game/Objects/Item/SuperMushroom.h"
#include "Game/Objects/Player/Player.h"

SuperMushroom::SuperMushroom() : Item() {
}

SuperMushroom::SuperMushroom(sf::Texture& texture) : Item() {
    animatable->configureVisuals(texture, "super_mushroom");
}

void SuperMushroom::onPickup(Player& player) {
    // Only upgrade Normal -> Super
    if (player.getState() && player.getState()->getStateName() == "Normal") {
        player.changeToSuperState();
    }

    //Pick up +1000 points if player is Super or Fire
    destroy();
}

void SuperMushroom::onCreateBodyDef(b2BodyDef& def) {
    def.type = b2_staticBody;
}

void SuperMushroom::onCreateShapeDef(b2ShapeDef& def) {
    def.isSensor = true;
    def.density = 0.0f;

    def.filter.categoryBits = 0x0010;
    def.filter.maskBits = 0x0002;
}

void SuperMushroom::onUpdateVisuals(float deltaTime) {
    Item::onUpdateVisuals(deltaTime);
}
