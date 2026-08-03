#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <SFML/System/Vector2.hpp>
#include <nlohmann/json.hpp>

enum class ObjectKind {
    Block,
    Player,
    Enemy,
    Item
};

struct SpawnSpec {
    ObjectKind kind = ObjectKind::Item;
    std::string typeKey;
    std::string animationId;
    std::string textureKey;
    sf::Vector2f size;
    sf::Vector2f offset;
    bool centerVertically = false;
    bool addSeamFilter = false; // = true for Block so that Player wont flickering 
    bool addController = false; // add controller for player
    std::shared_ptr<SpawnSpec> contents; // to contains other objects inside of it
};

void from_json(const nlohmann::json& json, SpawnSpec& spec);
std::unordered_map<int, std::vector<SpawnSpec>> parseSpawnSpecs(
    const nlohmann::json& json
);
