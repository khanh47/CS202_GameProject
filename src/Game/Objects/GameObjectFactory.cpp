#include <stdexcept>

#include "Game/Objects/GameObjectFactory.h"
#include "Game/Objects/Block/Block.h"
#include "Game/Objects/Enemy/Enemy.h"
#include "Game/Objects/Item/Item.h"
#include "Game/Objects/Player/Player.h"

GameObjectFactory::GameObjectFactory() {
    registerPlayer("Player", [this](sf::Texture* texture, const std::string& animationSetId) -> std::shared_ptr<GameObject> {
        std::shared_ptr<Player> returned_player;
        if (texture) {
            returned_player = std::make_shared<Player>(*texture, animationSetId);
        } else {
            returned_player = std::make_shared<Player>();
        }
        return returned_player;
    });

    registerBlock("Block", [this](sf::Texture* texture) -> std::shared_ptr<GameObject> {
        std::shared_ptr<Block> returned_block;
        if (texture) {
            returned_block = std::make_shared<Block>(*texture);
        } else {
            returned_block = std::make_shared<Block>();
        }
        return returned_block;
    });

    registerEnemy("Enemy", [this](sf::Texture* texture, const std::string& animationSetId) -> std::shared_ptr<GameObject> {
        std::shared_ptr<Enemy> returned_enemy;
        if (texture) {
            returned_enemy = std::make_shared<Enemy>(*texture, animationSetId);
        } else {
            returned_enemy = std::make_shared<Enemy>();
        }
        return returned_enemy;
    });

    registerItem("Item", [this](sf::Texture* texture) -> std::shared_ptr<GameObject> {
        std::shared_ptr<Item> returned_item;
        if (texture) {
            returned_item = std::make_shared<Item>(*texture);
        } else {
            returned_item = std::make_shared<Item>();
        }
        return returned_item;
    });
}

void GameObjectFactory::registerPlayer(const std::string& key, PlayerCreator creator) {
    _playerCreators[key] = std::move(creator);
}

void GameObjectFactory::registerBlock(const std::string& key, Creator creator) {
    _blockCreators[key] = std::move(creator);
}

void GameObjectFactory::registerEnemy(const std::string& key, PlayerCreator creator) {
    _enemyCreators[key] = std::move(creator);
}

void GameObjectFactory::registerItem(const std::string& key, Creator creator) {
    _itemCreators[key] = std::move(creator);
}

std::shared_ptr<GameObject> GameObjectFactory::createPlayer(const std::string& key, sf::Texture* texture, const std::string& animationSetId) const {
    const auto it = _playerCreators.find(key);
    if (it == _playerCreators.end()) {
        throw std::runtime_error("Unknown player type: " + key);
    }

    return it->second(texture, animationSetId);
}

std::shared_ptr<GameObject> GameObjectFactory::createBlock(const std::string& key, sf::Texture* texture) const {
    const auto it = _blockCreators.find(key);
    if (it == _blockCreators.end()) {
        throw std::runtime_error("Unknown block type: " + key);
    }

    return it->second(texture);
}

std::shared_ptr<GameObject> GameObjectFactory::createEnemy(const std::string& key, sf::Texture* texture, const std::string& animationSetId) const {
    const auto it = _enemyCreators.find(key);
    if (it == _enemyCreators.end()) {
        throw std::runtime_error("Unknown enemy type: " + key);
    }

    return it->second(texture, animationSetId);
}

std::shared_ptr<GameObject> GameObjectFactory::createItem(const std::string& key, sf::Texture* texture) const {
    const auto it = _itemCreators.find(key);
    if (it == _itemCreators.end()) {
        throw std::runtime_error("Unknown item type: " + key);
    }

    return it->second(texture);
}
