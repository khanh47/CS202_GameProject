#include "Game/Objects/Item/Coin.h"
#include "Game/Behaviours/Animatable.h"
#include "Game/Objects/Player/Player.h"

Coin::Coin() : Item() {
}

Coin::Coin(sf::Texture& texture) : Item() {
    if (auto* animatable = getBehaviour<Animatable>()) {
        animatable->configureVisuals(texture, "coin");
    }
}

void Coin::onPickup(Player& player) {
    player.changeToFireState();
    destroy();
}

void Coin::onCreateBodyDef(b2BodyDef& def) {
    def.type = b2_staticBody;
}

void Coin::onCreateShapeDef(b2ShapeDef& def) {
    def.isSensor = true;
    def.density = 0.0f;

    def.filter.categoryBits = 0x0010;
    def.filter.maskBits = 0x0002;
}

void Coin::onUpdateVisuals(float deltaTime) {
    Item::onUpdateVisuals(deltaTime);
}
