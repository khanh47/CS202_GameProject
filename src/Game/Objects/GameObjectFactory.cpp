#include <stdexcept>

#include "Game/Objects/GameObjectFactory.h"
#include "Game/Objects/Blocks/Block.h"
#include "Game/Objects/Player/Player.h"

GameObjectFactory::GameObjectFactory() {
    registerPlayer("Player", [this](sf::Texture* texture) -> std::shared_ptr<GameObject> {
        std::shared_ptr<Player> returned_player;
        if (texture) {
            returned_player = std::make_shared<Player>(*texture);
        }
        returned_player = std::make_shared<Player>();
        return returned_player;
    });

    registerBlock("Block", [this](sf::Texture* texture) -> std::shared_ptr<GameObject> {
        std::shared_ptr<Block> returned_block;
        if (texture) {
            returned_block = std::make_shared<Block>(*texture);
        }
        returned_block = std::make_shared<Block>();
        return returned_block;
    });
}

void GameObjectFactory::registerPlayer(const std::string& key, Creator creator) {
    _playerCreators[key] = std::move(creator);
}

void GameObjectFactory::registerBlock(const std::string& key, Creator creator) {
    _blockCreators[key] = std::move(creator);
}

std::shared_ptr<GameObject> GameObjectFactory::createPlayer(const std::string& key, sf::Texture* texture) const {
    const auto it = _playerCreators.find(key);
    if (it == _playerCreators.end()) {
        throw std::runtime_error("Unknown player type: " + key);
    }

    return it->second(texture);
}

std::shared_ptr<GameObject> GameObjectFactory::createBlock(const std::string& key, sf::Texture* texture) const {
    const auto it = _blockCreators.find(key);
    if (it == _blockCreators.end()) {
        throw std::runtime_error("Unknown block type: " + key);
    }

    return it->second(texture);
}