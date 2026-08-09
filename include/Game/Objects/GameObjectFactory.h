#pragma once

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

#include <SFML/Graphics.hpp>

#include "Game/Objects/GameObject.h"

class GameObjectFactory {
public:
    using Creator = std::function<std::shared_ptr<GameObject>(sf::Texture* texture)>;
    using AnimatedCreator = std::function<std::shared_ptr<GameObject>(sf::Texture* texture, const std::string& animationSetId)>;

    template<typename T>
    static std::shared_ptr<GameObject> createAnimated(sf::Texture* texture, const std::string& animId) {
        if (texture) return std::make_shared<T>(*texture, animId);
        return std::make_shared<T>();
    }

    template<typename T>
    static std::shared_ptr<GameObject> createStatic(sf::Texture* texture) {
        if (texture) return std::make_shared<T>(*texture);
        return std::make_shared<T>();
    }

    GameObjectFactory();

    void registerPlayer(const std::string& key, AnimatedCreator creator);
    void registerBlock(const std::string& key, Creator creator);
    void registerEnemy(const std::string& key, AnimatedCreator creator);
    void registerItem(const std::string& key, Creator creator);

    std::shared_ptr<GameObject> createPlayer(const std::string& key = "Player", sf::Texture* texture = nullptr, const std::string& animationSetId = "mario") const;
    std::shared_ptr<GameObject> createBlock(const std::string& key = "Block", sf::Texture* texture = nullptr) const;
    std::shared_ptr<GameObject> createEnemy(const std::string& key, sf::Texture* texture = nullptr, const std::string& animationSetId = "") const;
    std::shared_ptr<GameObject> createItem(const std::string& key = "Item", sf::Texture* texture = nullptr) const;
private:
    std::unordered_map<std::string, AnimatedCreator> _playerCreators;
    std::unordered_map<std::string, Creator> _blockCreators;
    std::unordered_map<std::string, AnimatedCreator> _enemyCreators;
    std::unordered_map<std::string, Creator> _itemCreators;
};
