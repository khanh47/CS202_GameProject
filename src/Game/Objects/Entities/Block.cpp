#include "Game/Objects/Entities/Block.h"
#include "ResourceManager.h"

Block::Block() : GameObject() {
    _sprite = sf::Sprite(ResourceManager::getInstance().getTexture("tiles"));

    if (_sprite.has_value()) {
        sf::FloatRect bounds = _sprite.value().getLocalBounds();
        _sprite->setOrigin({bounds.size.x / 2.f, bounds.size.y / 2.f});
    }
}

Block::Block(sf::Texture &texture) : GameObject() {
    _sprite = sf::Sprite(texture);
}

Block::~Block() {
}