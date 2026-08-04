#include <stdexcept>

#include "Game/Objects/GameObjectFactory.h"
#include "Game/Objects/Block/Block.h"
#include "Game/Objects/Enemy/ConcreteEnemy/Goomba.h"
#include "Game/Objects/Enemy/ConcreteEnemy/Koopa.h"
#include "Game/Objects/Item/Item.h"
#include "Game/Objects/Item/FireFlower.h"
#include "Game/Objects/Item/SuperMushroom.h"
#include "Game/Objects/Player/Player.h"
#include "Game/Objects/Item/Coin.h"

GameObjectFactory::GameObjectFactory() {
    registerPlayer("Player", createAnimated<Player>);
    registerBlock("Block", createStatic<Block>);
    registerEnemy("Goomba", createAnimated<Goomba>);
    registerEnemy("Koopa", createAnimated<Koopa>);
    registerItem("Item", createStatic<Item>);
    registerItem("FireFlower", createStatic<FireFlower>);
    registerItem("SuperMushroom", createStatic<SuperMushroom>);
    registerItem("Coin", createStatic<Coin>);
}

void GameObjectFactory::registerPlayer(const std::string& key, AnimatedCreator creator) {
    _playerCreators[key] = std::move(creator);
}

void GameObjectFactory::registerBlock(const std::string& key, Creator creator) {
    _blockCreators[key] = std::move(creator);
}

void GameObjectFactory::registerEnemy(const std::string& key, AnimatedCreator creator) {
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