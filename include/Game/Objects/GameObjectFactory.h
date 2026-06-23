#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include <SFML/Graphics.hpp>

#include "Game/Objects/GameObject.h"

class GameObjectFactory {
public:
    using Creator = std::function<std::shared_ptr<GameObject>(sf::Texture* texture)>;

    GameObjectFactory();

    void registerPlayer(const std::string& key, Creator creator);
    void registerBlock(const std::string& key, Creator creator);

    std::shared_ptr<GameObject> createPlayer(const std::string& key = "Player", sf::Texture* texture = nullptr) const;
    std::shared_ptr<GameObject> createBlock(const std::string& key = "Block", sf::Texture* texture = nullptr) const;

    void render(sf::RenderTarget& target);

private:
    std::vector<std::shared_ptr<GameObject>> _objects;
    std::unordered_map<std::string, Creator> _playerCreators;
    std::unordered_map<std::string, Creator> _blockCreators;
};
