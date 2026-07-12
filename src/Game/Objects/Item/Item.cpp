#include "Game/Objects/Item/Item.h"

Item::Item() : GameObject() {
}

Item::Item(sf::Texture& texture) : GameObject() {
    _sprite = sf::Sprite(texture);
}

Item::~Item() {
}
